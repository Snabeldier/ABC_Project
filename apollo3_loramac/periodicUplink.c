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
    #ifdef HW_ECOVETTE
    APP_TX_DUTYCYCLE = 120000;
    #elif HW_CF_SENSOR
    APP_TX_DUTYCYCLE = 60000; // Can change this to change duty cycle when needed or wanted
    #endif
    APP_TX_DUTYCYCLE = 10000; // Can change this to change duty cycle when needed or wanted
    TxPeriodicity = 0;
    while (TxPeriodicity < APP_TX_DUTYCYCLE || TxPeriodicity > APP_TX_DUTYCYCLE + APP_TX_DUTYCYCLE_RND) {
      TxPeriodicity = APP_TX_DUTYCYCLE + randr(0, APP_TX_DUTYCYCLE_RND);
    }

    //TxPeriodicity = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
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

	
//	// Copied from shtc3.c -> Timm Luhmann (IMTEK)
//typedef struct
//{
//    uint32_t                    ui32Module;
//    void                        *pIomHandle;
//    bool                        bOccupied;
//} am_devices_iom_shtc3_t;

//am_devices_iom_shtc3_t gShtc3[AM_DEVICES_SHTC3_MAX_DEVICE_NUM];	//one device maximum
//#include "shtc3.h"

//static etError SHTC3_CheckCrc(uint8_t data[], uint8_t nbrOfBytes,
//                              uint8_t checksum)
//{
//    uint8_t bit;        // bit mask
//    uint8_t crc = 0xFF; // calculated checksum
//    uint8_t byteCtr;    // byte counter

//    // calculates 8-Bit checksum with given polynomial
//    for (byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
//    {
//        crc ^= (data[byteCtr]);
//        for (bit = 8; bit > 0; --bit)
//        {
//            if (crc & 0x80)
//            {
//                crc = (crc << 1) ^ CRC_POLYNOMIAL;
//            }
//            else
//            {
//                crc = (crc << 1);
//            }
//        }
//    }

//    // verify checksum
//    if (crc != checksum)
//    {
//        return CHECKSUM_ERROR;
//    }
//    else
//    {
//        return NO_ERROR;
//    }
//}

// // end of copy
	
static etError MeasureTempAndHumidity(uint16_t *tempData, uint16_t *humData) {
	etError error = NO_ERROR;
//  uint8_t data[6];  // Array to hold data bytes and CRCs
//  uint8_t expectedCrc;

//  // I2C read sequence
//  am_devices_iom_shtc3_t *pIom = (am_devices_iom_shtc3_t *)my_IomdevHdl;
//  if (am_device_command_read(pIom->pIomHandle, 0x44, 0, 0, false, (uint32_t *)data, 6)) {
//		return ACK_ERROR;  // Error in command read
//  }

//  // Verify CRC for Temperature
//  expectedCrc = data[2];  // CRC for first two bytes
//  if (SHTC3_CheckCrc(data, 2, expectedCrc) != NO_ERROR) {
//      error |= CHECKSUM_ERROR;
//  } else {
//      *tempData = (data[0] << 8) | data[1];  // Combine bytes into a 16-bit value
//  }

//  // Verify CRC for Humidity
//  expectedCrc = data[5];  // CRC for next two bytes
//  if (SHTC3_CheckCrc(&data[3], 2, expectedCrc) != NO_ERROR) {
//      error |= CHECKSUM_ERROR;
//  } else {
//      *humData = (data[3] << 8) | data[4];  // Combine bytes into a 16-bit value
//  }

  return error;
}

/*!
 * Prepares the payload of the frame and transmits it.
 */
static void PrepareTxFrame(void) {

  /*!
   * User application data
   */
  uint8_t AppDataBuffer2[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

  /*!
   * User application data structure
   */
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

  //uint8_t channel = 0;

  //SendData_t data;

  //data.Measurement.fl_ctr = fl_meas_ctr;

  //measurementProcess( & data); //get measurement data #todo implement this

  //fl_meas_ctr = data.Measurement.fl_ctr;

  AppData2.Port = LORAWAN_APP_PORT;
  JalapenosLppReset();
  JalapenosLppAddDeviceID(34000);
  JalapenosLppAddDummyData();
	am_util_stdio_printf("[DEBUG] Start Data Collection\n");
	//uint16_t temp;
	//uint16_t hum;
	float temp;
	float hum;
	etError error = SHTC3_GetTempAndHumi(&temp, &hum);
	if (error == ACK_ERROR) {
		am_util_stdio_printf("[ERROR] Error while reading sensor data: ACK_ERROR\n");
	}
	if (error == CHECKSUM_ERROR) {
		am_util_stdio_printf("[ERROR] Error while reading sensor data: CHECKSUM_ERROR\n");
	}
	//MeasureTempAndHumidity(&temp, &hum);
	//am_util_stdio_printf("[DEBUG] %d %d", &temp, &hum);
	JalapenosLppAddTemperatureAndHumidity(temp, hum);

  //SysTime_t currentTime = SysTimeGet();
  //JalapenosLppAddTimestamp(channel, currentTime.Seconds);

  am_util_stdio_printf("[DEBUG] Data sending...");
  JalapenosLppCopy(AppData2.Buffer);
  AppData2.BufferSize = JalapenosLppGetSize();

  //LmHandlerDeviceTimeReq();
  LmHandlerSend( & AppData2, LmHandlerParams.IsTxConfirmed);
  am_util_stdio_printf("[DEBUG] Data successfully sent!");
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