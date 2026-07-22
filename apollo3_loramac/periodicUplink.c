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
#include "ina219.h"
#include "deviceConfig.h"

/*!
 * LoRaWAN default end-device class
 */
#ifndef LORAWAN_DEFAULT_CLASS
#define LORAWAN_DEFAULT_CLASS CLASS_A
#endif

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */

//changing this seems to have no real effect on the TX timing. Next step is to figure out the timing for transmission and receive because thats whats currently missaligned
//debug and check where the TX timer is set and with what & where this value originates, same for RX
uint32_t APP_TX_DUTYCYCLE = 2000;

uint32_t fl_meas_ctr = 4;

/*!
 * Supply-current/voltage capture (INA219), dumped as JSON over SWO/ITM.
 *
 * The INA219 is sampled continuously into a ring buffer, so the samples just
 * *before* a measurement are always available. When a measurement/transmission
 * starts, PRE_SAMPLES of history plus POST_SAMPLES taken afterwards (covering the
 * sensor read, the TX, the RX windows and a bit of idle after) are captured and
 * then printed once as a single JSON object over SWO (am_util_stdio_printf,
 * captured on the PC with the J-Link SWO viewer / Keil printf viewer).
 *
 * Each sample carries a real timestamp measured from the Apollo3 STIMER, a
 * free-running 32.768 kHz counter (~30.5 us resolution, keeps running in sleep),
 * reported as microseconds relative to the first sample of the capture. Fields
 * per sample (all signed integers):
 *   t_us       : time since the first captured sample (us)
 *   current_uA : current in microamperes (mA = value / 1000)
 *   shunt_uV   : shunt voltage in microvolts (mV = value / 1000)
 *   bus_mV     : bus voltage in millivolts (V = value / 1000)
 *
 * The LoRa uplink keeps sending only temperature and humidity.
 */
#define SAMPLE_INTERVAL  10                            /* ms between samples          */
#define PRE_SAMPLES      50                            /* history kept before trigger */
#define POST_SAMPLES     250                           /* samples after the trigger   */
#define CAPTURE_MAX      (PRE_SAMPLES + POST_SAMPLES)  /* ring buffer size            */
#define STIMER_HZ        32768UL                       /* STIMER clock (XTAL 32 kHz)  */

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
#define LORAWAN_ADR_STATE LORAMAC_HANDLER_ADR_ON //can change to OFF

/*!
 * Default datarate
 *
 * \remark Please note that LORAWAN_DEFAULT_DATARATE is used only when ADR is disabled 
 */
#define LORAWAN_DEFAULT_DATARATE DR_0 //can change to 3

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
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxTimer;

/*!
 * Green LED (pin 38, configured as output in main.c): pulsed briefly whenever
 * an uplink is started. The one-shot LedTimer turns it off again so the pulse
 * does not block the MAC processing.
 */
#define GREEN_LED_PIN 38
#define LED_PULSE_MS  100

static TimerEvent_t LedTimer;

static void OnLedTimerEvent(void * context) {
  TimerStop( & LedTimer);
  am_hal_gpio_state_write(GREEN_LED_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
}

/*!
 * Ring buffer of captured samples (STIMER tick timestamp + INA219 readings).
 */
static uint32_t CapTicks[CAPTURE_MAX];       /* STIMER counter (32.768 kHz) */
static int32_t  CapCurrent_uA[CAPTURE_MAX];  /* current (uA)               */
static int32_t  CapShunt_uV[CAPTURE_MAX];    /* shunt voltage (uV)         */
static int32_t  CapBus_mV[CAPTURE_MAX];      /* bus voltage (mV)           */
static uint16_t CapHead = 0;                 /* next write index           */
static uint16_t CapFill = 0;                 /* valid samples (0..MAX)     */

/*!
 * Capture control. PostRoll counts the samples still to be taken after a trigger;
 * when it reaches 0 the capture is complete and IsDumpPending requests the JSON
 * dump. TriggerTicks/TxTicks are STIMER timestamps of the measurement start and
 * the LmHandlerSend call, reported in the JSON so the TX can be located.
 */
static bool              CaptureActive = false;
static uint16_t          PostRoll = 0;
static volatile uint8_t  IsDumpPending = 0;
static uint32_t          TriggerTicks = 0;
static uint32_t          TxTicks = 0;

/*!
 * Timer that paces the continuous sampling (one sample every SAMPLE_INTERVAL).
 */
static TimerEvent_t SampleTimer;

/*!
 * Set in the sample timer callback (ISR), handled in the main loop.
 */
static volatile uint8_t IsSamplePending = 0;

/*!
 * True once the INA219 has been initialised successfully.
 */
static bool Ina219Ready = false;

/*
 * Function that reads the external RTC and checks if there is a timestamp available 
 */
static bool rtcRead();

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent(void * context);

/*!
 * Sample timer event (ISR): flags a sample every SAMPLE_INTERVAL.
 */
static void OnSampleTimerEvent(void * context);

/*!
 * Reads one INA219 sample into the ring buffer in the main loop when flagged.
 */
static void SampleProcess(void);

/*!
 * Prints the captured window as a single JSON object over SWO.
 */
static void DumpCaptureJson(void);

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
  .FwVersion.Value = FIRMWARE_VERSION,
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

  const Version_t appVersion = {
    .Value = FIRMWARE_VERSION
  };
  const Version_t gitHubVersion = {
    .Value = GITHUB_VERSION
  };
  DisplayAppInfo("periodic-uplink-lpp", &
    appVersion, &
    gitHubVersion);

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

  // Set system maximum tolerated rx error in milliseconds
  LmHandlerSetSystemMaxRxError(25);

  // The LoRa-Alliance Compliance protocol package should always be
  // initialized and activated.
  LmHandlerPackageRegister(PACKAGE_ID_COMPLIANCE, & LmhpComplianceParams);

  LmHandlerJoin();
  //once everything is initialized, the system checks if a Device Time Request needs to be executed before measurements can start
  //requireTimeRequest = rtcRead();

  StartTxProcess(LORAMAC_HANDLER_TX_ON_TIMER);

  // Start the STIMER as a free-running 32.768 kHz counter for high-resolution
  // (~30.5 us) sample timestamps. It keeps running in sleep and is independent
  // of the LoRaMac RTC. Not used elsewhere in this build.
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ);

  // One-shot timer that ends the green LED uplink pulse.
  TimerInit( & LedTimer, OnLedTimerEvent);
  TimerSetValue( & LedTimer, LED_PULSE_MS);

  // Initialise the INA219 and start the continuous sampling into the ring
  // buffer. SampleProcess() reads one sample per SampleTimer tick.
  if (INA219_Init() == INA219_NO_ERROR) {
    Ina219Ready = true;
    TimerInit( & SampleTimer, OnSampleTimerEvent);
    TimerSetValue( & SampleTimer, SAMPLE_INTERVAL);
    TimerStart( & SampleTimer);
  } else {
    am_util_stdio_printf("[ERROR] INA219 init failed: ACK_ERROR\n");
  }

  //LmHandlerDeviceTimeReq();

  while (1) {
    // Process characters sent over the command line interface
    //CliProcess( &Uart2 );

    // Processes the LoRaMac events
    LmHandlerProcess();

    // Process application uplinks management
    UplinkProcess();

    // Advance the INA219 capture; dump the window as JSON once complete
    SampleProcess();
    if (IsDumpPending) {
      DumpCaptureJson();
    }

    CRITICAL_SECTION_BEGIN();
    if (IsMacProcessPending == 1) {
      // Clear flag and prevent MCU to go into low power modes.
      IsMacProcessPending = 0;
    } else {
      // The MCU wakes up through events
      BoardLowPowerHandler();
    }
    CRITICAL_SECTION_END();
    APP_TX_DUTYCYCLE = 10000; // Can change this to change duty cycle when needed or wanted
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
}

static void OnRxData(LmHandlerAppData_t * appData, LmHandlerRxParams_t * params) {
  DisplayRxUpdate(appData, params);

  switch (appData -> Port) {
  case 1: // The application LED can be controlled on port 1 or 2
  case LORAWAN_APP_PORT: {
    AppLedStateOn = appData -> Buffer[0] & 0x01;
  }
  break;
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
 * Sample timer event. Runs in ISR context: only flag a sample and rearm so the
 * sampling runs continuously for the whole session.
 */
static void OnSampleTimerEvent(void * context) {
  TimerStop( & SampleTimer);
  IsSamplePending = 1;
  TimerSetValue( & SampleTimer, SAMPLE_INTERVAL);
  TimerStart( & SampleTimer);
}

/*!
 * Reads one INA219 sample whenever the timer flags it and stores it (with a chip
 * timestamp) in the ring buffer. While a capture is running, counts down the
 * post-trigger samples and requests the JSON dump once they are collected.
 * Runs in the main loop (thread context), so the blocking I2C read is fine.
 */
static void SampleProcess(void) {
  if (!IsSamplePending) {
    return;
  }
  IsSamplePending = 0;

  if (!Ina219Ready) {
    return;
  }

  float shuntVoltage_mV, busVoltage_V, current_uA;
  if (INA219_GetMeasurements( & shuntVoltage_mV, & busVoltage_V, & current_uA) != INA219_NO_ERROR) {
    return;
  }

  CapTicks[CapHead]      = am_hal_stimer_counter_get();
  CapCurrent_uA[CapHead] = (int32_t)(current_uA);              /* uA */
  CapShunt_uV[CapHead]   = (int32_t)(shuntVoltage_mV * 1000.0f); /* uV */
  CapBus_mV[CapHead]     = (int32_t)(busVoltage_V * 1000.0f);  /* mV */
  CapHead = (CapHead + 1) % CAPTURE_MAX;
  if (CapFill < CAPTURE_MAX) {
    CapFill++;
  }

  // Count down the post-trigger samples; request the dump when the window is full
  if (CaptureActive) {
    if (PostRoll > 0) {
      PostRoll--;
    }
    if (PostRoll == 0) {
      CaptureActive = false;
      IsDumpPending = 1;
    }
  }
}

/*!
 * Converts a STIMER tick count, relative to t0, into microseconds.
 */
static uint32_t TicksToUs(uint32_t ticks, uint32_t t0) {
  return (uint32_t)(((uint64_t)(ticks - t0) * 1000000ULL) / STIMER_HZ);
}

/*!
 * Prints the captured ring buffer (oldest -> newest) as a single JSON object
 * over SWO. Timestamps are reported in microseconds relative to the first
 * captured sample.
 */
static void DumpCaptureJson(void) {
  uint16_t n = CapFill;
  uint16_t oldest = (uint16_t)((CapHead + CAPTURE_MAX - n) % CAPTURE_MAX);
  uint32_t t0 = CapTicks[oldest];

  am_util_stdio_printf("{\"trigger_t_us\":%u,\"tx_t_us\":%u,\"samples\":[\n",
                       (unsigned)TicksToUs(TriggerTicks, t0),
                       (unsigned)TicksToUs(TxTicks, t0));

  for (uint16_t k = 0; k < n; k++) {
    uint16_t idx = (uint16_t)((oldest + k) % CAPTURE_MAX);
    am_util_stdio_printf("{\"t_us\":%u,\"current_uA\":%d,\"shunt_uV\":%d,\"bus_mV\":%d}%s\n",
                         (unsigned)TicksToUs(CapTicks[idx], t0),
                         (int)CapCurrent_uA[idx],
                         (int)CapShunt_uV[idx],
                         (int)CapBus_mV[idx],
                         (k + 1 < n) ? "," : "");
  }

  am_util_stdio_printf("]}\n");
  IsDumpPending = 0;
}

/*!
 * Prepares the payload of the frame and transmits it.
 */
static void PrepareTxFrame(void) {

  //User application data
  uint8_t AppDataBuffer2[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

  // User application data structure
  LmHandlerAppData_t AppData2 = {
    .Buffer = AppDataBuffer2,
    .BufferSize = 0,
    .Port = 0,
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

  // Trigger a current/voltage capture at the start of the measurement. The ring
  // buffer already holds PRE_SAMPLES of history (the "before"); from here the
  // sampler collects POST_SAMPLES more (sensor read, TX, RX windows, a bit after)
  // and then dumps the whole window as JSON.
  if (!CaptureActive && !IsDumpPending) {
    TriggerTicks  = am_hal_stimer_counter_get();
    PostRoll      = POST_SAMPLES;
    CaptureActive = true;
  }

  AppData2.Port = LORAWAN_APP_PORT;
  JalapenosLppReset();
  JalapenosLppAddDeviceID(DeviceConfigGet()->PayloadDeviceId);

	float temp;
	float hum;
	SHT3X_Error error = SHT3X_GetTempAndHumi(&temp, &hum);
	//etError error = SHTC3_GetTempAndHumi(&temp, &hum);
	if (error == SHT3X_ACK_ERROR) {
		am_util_stdio_printf("[ERROR] Error while reading sensor data: ACK_ERROR\n");
	}
	if (error == SHT3X_CHECKSUM_ERROR) {
		am_util_stdio_printf("[ERROR] Error while reading sensor data: CHECKSUM_ERROR\n");
	}
	JalapenosLppAddTemperatureAndHumidity(temp, hum);

  // The current/voltage trace is no longer sent over LoRa; it is captured into
  // the ring buffer and dumped as JSON over SWO. The LoRa uplink carries only
  // device ID, temperature and humidity.
  JalapenosLppCopy(AppData2.Buffer);
  AppData2.BufferSize = JalapenosLppGetSize();

  // Record the chip time of the transmission so the JSON can mark where the TX
  // (and thus the current peak) sits within the captured window.
  TxTicks = am_hal_stimer_counter_get();

  // Signal the started uplink with a short green LED pulse (turned off again
  // by OnLedTimerEvent).
  am_hal_gpio_state_write(GREEN_LED_PIN, AM_HAL_GPIO_OUTPUT_SET);
  TimerStart( & LedTimer);

  LmHandlerSend( & AppData2, LmHandlerParams.IsTxConfirmed);
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