/*!
 * \file      periodicUplink.c
 *
 * \brief     Performs a periodic uplink
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Timm Luhmann (IMTEK)
 */

#include "periodicUplink.h"

#include "am_util_stdio.h"	//required for printf operations on console

#include "am_util_delay.h"

#include "timing.h"

#include "sht3x.h"
#include "deviceConfig.h"
#include "bundle.h"

/*!
 * LoRaWAN default end-device class
 */
#ifndef LORAWAN_DEFAULT_CLASS
#define LORAWAN_DEFAULT_CLASS CLASS_A
#endif

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */

// The TX/RX timing misalignment observed here was traced to the Apollo3_V2
// rtc-board.c: RtcGetCalendarValue() dropped the hundredths (1 s timer
// granularity) and RtcStartAlarm() re-based alarms on "now" instead of the
// timer context, so the RX windows opened seconds late. Fixed there.

/* Measurement interval in ms — single definition for the entire application. */
#define T_MEASURE_MS 5000

uint32_t APP_TX_DUTYCYCLE = T_MEASURE_MS;

uint32_t fl_meas_ctr = 4;

/*!
 * Defines a random delay for application data transmission duty cycle. 1s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND 1000

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE LORAMAC_HANDLER_ADR_OFF

/*!
 * Default datarate
 *
 * \remark Please note that LORAWAN_DEFAULT_DATARATE is used only when ADR is disabled 
 */
#define LORAWAN_DEFAULT_DATARATE DR_3

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE LORAMAC_HANDLER_CONFIRMED_MSG

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE 242

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON true

/*!
 * LoRaWAN application port
 * @remark The allowed port range is from 1 up to 223. Other values are reserved.
 */
#define LORAWAN_APP_PORT 2

/*!
 *
 */
typedef enum {
  LORAMAC_HANDLER_TX_ON_TIMER,
  LORAMAC_HANDLER_TX_ON_EVENT,
}
LmHandlerTxEvents_t;

/*!
 * User application data
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/*!
 * User application data structure
 */
static LmHandlerAppData_t AppData = {
  .Buffer = AppDataBuffer,
  .BufferSize = 0,
  .Port = 0,
};

/*!
 * Specifies the state of the application LED
 */
static bool AppLedStateOn = false;

/*!
 * Numeric value last received via downlink on port 2; echoed back in every uplink.
 */
static uint8_t AppNumericValue = 0;

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxTimer;

/*!
 * Red LED (pin 37, configured as output in main.c): pulsed briefly whenever
 * an uplink is started. The one-shot LedTimer turns it off again so the pulse
 * does not block the MAC processing.
 */
#define RED_LED_PIN  37
#define LED_PULSE_MS 100

/*!
 * Green LED (pin 38, configured as output in main.c): switched on/off by a
 * downlink on port 1 or 2 (payload byte 0, bit 0).
 */
#define GREEN_LED_PIN 38

static TimerEvent_t LedTimer;

static void OnLedTimerEvent(void * context) {
  TimerStop( & LedTimer);
  am_hal_gpio_state_write(RED_LED_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
}

/*
 * Function that reads the external RTC and checks if there is a timestamp available 
 */
static bool rtcRead();

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent(void * context);

static void OnMacProcessNotify(void);
static void OnNvmDataChange(LmHandlerNvmContextStates_t state, uint16_t size);
static void OnNetworkParametersChange(CommissioningParams_t * params);
static void OnMacMcpsRequest(LoRaMacStatus_t status, McpsReq_t * mcpsReq, TimerTime_t nextTxIn);
static void OnMacMlmeRequest(LoRaMacStatus_t status, MlmeReq_t * mlmeReq, TimerTime_t nextTxIn);
static void OnJoinRequest(LmHandlerJoinParams_t * params);
static void OnTxData(LmHandlerTxParams_t * params);
static void OnRxData(LmHandlerAppData_t * appData, LmHandlerRxParams_t * params);
static void OnClassChange(DeviceClass_t deviceClass);
static void OnBeaconStatusChange(LoRaMacHandlerBeaconParams_t * params);
#if(LMH_SYS_TIME_UPDATE_NEW_API == 1)
static void OnSysTimeUpdate(bool isSynchronized, int32_t timeCorrection);
#else
static void OnSysTimeUpdate(void);
#endif
static void PrepareTxFrame(void);
static void StartTxProcess(LmHandlerTxEvents_t txEvent);
static void UplinkProcess(void);

static void OnTxPeriodicityChanged(uint32_t periodicity);
static void OnTxFrameCtrlChanged(LmHandlerMsgTypes_t isTxConfirmed);
static void OnPingSlotPeriodicityChanged(uint8_t pingSlotPeriodicity);

static LmHandlerCallbacks_t LmHandlerCallbacks = {
  .GetBatteryLevel = BoardGetBatteryLevel,
  .GetTemperature = NULL,
  .GetRandomSeed = BoardGetRandomSeed,
  .OnMacProcess = OnMacProcessNotify,
  .OnNvmDataChange = OnNvmDataChange,
  .OnNetworkParametersChange = OnNetworkParametersChange,
  .OnMacMcpsRequest = OnMacMcpsRequest,
  .OnMacMlmeRequest = OnMacMlmeRequest,
  .OnJoinRequest = OnJoinRequest,
  .OnTxData = OnTxData,
  .OnRxData = OnRxData,
  .OnClassChange = OnClassChange,
  .OnBeaconStatusChange = OnBeaconStatusChange,
  .OnSysTimeUpdate = OnSysTimeUpdate,
};

static LmHandlerParams_t LmHandlerParams = {
  .Region = ACTIVE_REGION,
  .AdrEnable = LORAWAN_ADR_STATE,
  .IsTxConfirmed = LORAWAN_DEFAULT_CONFIRMED_MSG_STATE,
  .TxDatarate = LORAWAN_DEFAULT_DATARATE,
  .PublicNetworkEnable = LORAWAN_PUBLIC_NETWORK,
  .DutyCycleEnabled = LORAWAN_DUTYCYCLE_ON,
  .DataBufferMaxSize = LORAWAN_APP_DATA_BUFFER_MAX_SIZE,
  .DataBuffer = AppDataBuffer,
  .PingSlotPeriodicity = REGION_COMMON_DEFAULT_PING_SLOT_PERIODICITY,
};

static LmhpComplianceParams_t LmhpComplianceParams = {
  .FwVersion.Value = 0x1300,
  .OnTxPeriodicityChanged = OnTxPeriodicityChanged,
  .OnTxFrameCtrlChanged = OnTxFrameCtrlChanged,
  .OnPingSlotPeriodicityChanged = OnPingSlotPeriodicityChanged,
};

/*!
 * Indicates if LoRaMacProcess call is pending.
 * 
 * \warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static volatile uint8_t IsMacProcessPending = 0;

static volatile uint8_t IsTxFramePending = 0;

static volatile uint32_t TxPeriodicity = 0;

volatile bool requireTimeRequest = false;

/*!
 * Main application entry point.
 */
void periodicUplink(void) {
  BoardInitMcu();
  BoardInitPeriph();

  if (!gpioRead(ADC_POWEREXTRA)) {
    NvmDataMgmtFactoryReset(); //reset nvm storage
  }

  am_util_delay_ms(12);

  // Initialize transmission periodicity variable
  TxPeriodicity = APP_TX_DUTYCYCLE + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);

  const Version_t gitHubVersion = {
    .Value = GITHUB_VERSION
  };

  // Show which per-node configuration was selected (see deviceConfig.c). The
  // ChipID0 value printed here is what goes into the DeviceConfigTable entry.
  {
    am_hal_mcuctrl_device_t device;
    am_hal_mcuctrl_info_get(AM_HAL_MCUCTRL_INFO_DEVICEID, &device);
    am_util_stdio_printf("[INFO] ChipID0 = 0x%08X, DeviceID = %u, DevAddr = 0x%08X\n",
                         device.ui32ChipID0,
                         (unsigned int)DeviceConfigGet()->PayloadDeviceId,
                         DeviceConfigGet()->DevAddr);
  }

  if (LmHandlerInit( & LmHandlerCallbacks, & LmHandlerParams) != LORAMAC_HANDLER_SUCCESS) {
    am_util_stdio_printf("LoRaMac wasn't properly initialized\n");
    // Fatal error, endless loop.
    while (1) {}
  }

  {
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_CHANNELS_NB_TRANS;
    mibReq.Param.ChannelsNbTrans = 15;
    LoRaMacMibSetRequestConfirm(&mibReq);
  }

  Bundle_Init();

  // Set system maximum tolerated rx error in milliseconds. The Apollo3 RTC
  // timebase has a 10 ms tick (100 Hz), so alarm scheduling and the ms->tick
  // conversions can each be off by up to one tick; 50 ms keeps the RX windows
  // wide enough to absorb that.
  LmHandlerSetSystemMaxRxError(50);

  // ABP: the RX window delays are never negotiated with the network (no join
  // accept). The TTN live data shows this session schedules downlinks with
  // rx1_delay = 1 s, which matches the LoRaMac regional default (RX1 1 s,
  // RX2 2 s) -- so the defaults are kept as they are.

  // The LoRa-Alliance Compliance protocol package should always be
  // initialized and activated.
  LmHandlerPackageRegister(PACKAGE_ID_COMPLIANCE, & LmhpComplianceParams);

  LmHandlerJoin();
  //once everything is initialized, the system checks if a Device Time Request needs to be executed before measurements can start
  //requireTimeRequest = rtcRead();

  StartTxProcess(LORAMAC_HANDLER_TX_ON_TIMER);

  // One-shot timer that ends the green LED uplink pulse.
  TimerInit( & LedTimer, OnLedTimerEvent);
  TimerSetValue( & LedTimer, LED_PULSE_MS);

  //LmHandlerDeviceTimeReq();

  while (1) {
    // Process characters sent over the command line interface
    //CliProcess( &Uart2 );

    // Processes the LoRaMac events
    LmHandlerProcess();

    // Process application uplinks management
    UplinkProcess();

    CRITICAL_SECTION_BEGIN();
    if (IsMacProcessPending == 1) {
      // Clear flag and prevent MCU to go into low power modes.
      IsMacProcessPending = 0;
    } else {
      // The MCU wakes up through events
      BoardLowPowerHandler();
    }
    CRITICAL_SECTION_END();
    APP_TX_DUTYCYCLE = T_MEASURE_MS;
    TxPeriodicity = 0;
    while (TxPeriodicity < APP_TX_DUTYCYCLE || TxPeriodicity > APP_TX_DUTYCYCLE + APP_TX_DUTYCYCLE_RND) {
      TxPeriodicity = APP_TX_DUTYCYCLE + randr(0, APP_TX_DUTYCYCLE_RND);
    }
  }
}

static bool rtcRead() {
  time_struct currentTime;
  am_devices_am1805_get_time( & currentTime);
  if (currentTime.year == 0) { //all time registers are reset 0, so making sure by taking the largest one to have a valid return value
    return true;
  } else {
    return false;
  }
}

static void OnMacProcessNotify(void) {
  IsMacProcessPending = 1;
}

static void OnNvmDataChange(LmHandlerNvmContextStates_t state, uint16_t size) {
  DisplayNvmDataChange(state, size);
}

static void OnNetworkParametersChange(CommissioningParams_t * params) {
  DisplayNetworkParametersUpdate(params);
}

static void OnMacMcpsRequest(LoRaMacStatus_t status, McpsReq_t * mcpsReq, TimerTime_t nextTxIn) {
  DisplayMacMcpsRequestUpdate(status, mcpsReq, nextTxIn);
}

static void OnMacMlmeRequest(LoRaMacStatus_t status, MlmeReq_t * mlmeReq, TimerTime_t nextTxIn) {
  DisplayMacMlmeRequestUpdate(status, mlmeReq, nextTxIn);
}

static void OnJoinRequest(LmHandlerJoinParams_t * params) {
  DisplayJoinRequestUpdate(params);
  if (params -> Status == LORAMAC_HANDLER_ERROR) {
    LmHandlerJoin();
  } else {
    LmHandlerRequestClass(LORAWAN_DEFAULT_CLASS);
  }
}

static void OnTxData(LmHandlerTxParams_t * params) {
  DisplayTxUpdate(params);

  if (params->IsMcpsConfirm) {
    Bundle_OnTxDone(params->AckReceived != 0);
  }
}

// Wird automatisch aufgerufen, sobald ein RX empfangen wurde.
static void OnRxData(LmHandlerAppData_t * appData, LmHandlerRxParams_t * params) {
  DisplayRxUpdate(appData, params);

  switch (appData -> Port) {
  case 1: {
    if (appData -> BufferSize > 0) {
      AppLedStateOn = (appData -> Buffer[0] & 0x01) != 0;
      am_hal_gpio_state_write(GREEN_LED_PIN, AppLedStateOn ? AM_HAL_GPIO_OUTPUT_SET
                                                           : AM_HAL_GPIO_OUTPUT_CLEAR);
    }
    break;
  }
  case LORAWAN_APP_PORT: {
    if (appData -> BufferSize > 0) {
      AppNumericValue = appData -> Buffer[0];
    }
    break;
  }
  default:
    break;
  }
}

static void OnClassChange(DeviceClass_t deviceClass) {
  DisplayClassUpdate(deviceClass);

  // Inform the server as soon as possible that the end-device has switched to ClassB
  LmHandlerAppData_t appData = {
    .Buffer = NULL,
    .BufferSize = 0,
    .Port = 0,
  };
  LmHandlerSend( & appData, LORAMAC_HANDLER_UNCONFIRMED_MSG);
}

static void OnBeaconStatusChange(LoRaMacHandlerBeaconParams_t * params) {
  switch (params -> State) {
  case LORAMAC_HANDLER_BEACON_RX: {

    break;
  }
  case LORAMAC_HANDLER_BEACON_LOST:
  case LORAMAC_HANDLER_BEACON_NRX: {

    break;
  }
  default: {
    break;
  }
  }

  DisplayBeaconUpdate(params);
}

#if(LMH_SYS_TIME_UPDATE_NEW_API == 1)
static void OnSysTimeUpdate(bool isSynchronized, int32_t timeCorrection) {}
#else
static void OnSysTimeUpdate(void) {}
#endif

/*!
 * Prepares the payload of the frame and transmits it.
 *
 * Called on every TxTimer tick (T_MEASURE_MS).  Reads the SHT31 sensor and
 * adds the sample to the bundle buffer.  Returns without transmitting until
 * the buffer reaches N_BUNDLE samples.  Once full, serialises and calls
 * LmHandlerSend.  The buffer is cleared in Bundle_OnTxDone after MCPS-Confirm.
 */
static void PrepareTxFrame(void) {
  uint8_t txBuffer[2 + 4 * N_BUNDLE_MAX];
  LmHandlerAppData_t txData = {
    .Buffer     = txBuffer,
    .BufferSize = 0,
    .Port       = LORAWAN_APP_PORT,
  };

  if (LmHandlerIsBusy() == true) {
    return;
  }

  if (requireTimeRequest == true) {
    AppData.Buffer = 0;
    AppData.BufferSize = 0;
    AppData.Port = 0;
    LmHandlerDeviceTimeReq();
    LmHandlerSend( & AppData, LmHandlerParams.IsTxConfirmed);
    return;
  }

  if (!Bundle_IsTxReady()) {
    /* Normal measurement cycle: read sensor and add to bundle. */
    uint16_t temp_raw, hum_raw;
    SHT3X_Error error = SHT3X_GetRaw(&temp_raw, &hum_raw);
    if (error != SHT3X_NO_ERROR) {
      am_util_stdio_printf("[ERROR] SHT3X_GetRaw: %d\n", (int)error);
      return; /* skip measurement on I2C or CRC error */
    }
    if (!Bundle_AddSample(temp_raw, hum_raw)) {
      return; /* bundle not yet full: sleep until next timer tick */
    }
    /* AddSample promoted the buffer to IN_FLIGHT; fall through to TX. */
  }

  txData.BufferSize = Bundle_SerializeInFlight(txBuffer);
  LmHandlerSend( & txData, LORAMAC_HANDLER_CONFIRMED_MSG);
}

static void StartTxProcess(LmHandlerTxEvents_t txEvent) {
  switch (txEvent) {
  default:
    // Intentional fall through
  case LORAMAC_HANDLER_TX_ON_TIMER: {
    // Schedule 1st packet transmission

    TimerInit( & TxTimer, OnTxTimerEvent);
    TimerSetValue( & TxTimer, TxPeriodicity);
    OnTxTimerEvent(NULL);
  }
  break;
  case LORAMAC_HANDLER_TX_ON_EVENT: {}
  break;
  }
}

static void UplinkProcess(void) {
  uint8_t isPending = 0;
  CRITICAL_SECTION_BEGIN();
  isPending = IsTxFramePending;
  IsTxFramePending = 0;
  CRITICAL_SECTION_END();
  if (isPending == 1) {
    PrepareTxFrame();
  }
}

static void OnTxPeriodicityChanged(uint32_t periodicity) {
  TxPeriodicity = periodicity;

  if (TxPeriodicity == 0) { // Revert to application default periodicity
    TxPeriodicity = APP_TX_DUTYCYCLE + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
  }

  // Update timer periodicity
  TimerStop( & TxTimer);
  TimerSetValue( & TxTimer, TxPeriodicity);
  TimerStart( & TxTimer);
}

static void OnTxFrameCtrlChanged(LmHandlerMsgTypes_t isTxConfirmed) {
  LmHandlerParams.IsTxConfirmed = isTxConfirmed;
}

static void OnPingSlotPeriodicityChanged(uint8_t pingSlotPeriodicity) {
  LmHandlerParams.PingSlotPeriodicity = pingSlotPeriodicity;
}

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent(void * context) {
  TimerStop( & TxTimer);

  IsTxFramePending = 1;

  // Schedule next transmission
  TimerSetValue( & TxTimer, TxPeriodicity);
  TimerStart( & TxTimer);
}