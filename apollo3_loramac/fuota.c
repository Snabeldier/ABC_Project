/*!
 * \file      fuota.c
 *
 * \brief     FUOTA interop tests - test 01
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
 */


#include <stdio.h>
#include "../../common/githubVersion.h"
#include "utilities.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "RegionCommon.h"
#include "adc.h"
#include "transmitConfig.h"
#include "fuota.h"
#include "pins.h"

#include "sx126x.h"
#include "sx126x-board.h"

#include "radio.h"

#include "am_util_stdio.h"	//required for printf operations on console






/*!
 * User application data
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxTimer;

/*!
 * Timer to handle the state of LED1
 */
static TimerEvent_t Led1Timer;

/*!
 * Timer to handle the state of LED2
 */
static TimerEvent_t Led2Timer;

/*!
 * Timer to handle the state of LED beacon indicator
 */
static TimerEvent_t LedBeaconTimer;



static LmHandlerCallbacks_t LmHandlerCallbacks =
{
    .GetBatteryLevel = BoardGetBatteryLevel,
    //.GetTemperature = getAdcTemperature,
    .GetRandomSeed = BoardGetRandomSeed,
    .OnMacProcess = OnMacProcessNotify,
    .OnNvmDataChange = OnNvmDataChange,
    .OnNetworkParametersChange = OnNetworkParametersChange,
    .OnMacMcpsRequest = OnMacMcpsRequest,
    .OnMacMlmeRequest = OnMacMlmeRequest,
    .OnJoinRequest = OnJoinRequest,
    .OnTxData = OnTxData,
    .OnRxData = OnRxData,
    .OnClassChange= OnClassChange,
    .OnBeaconStatusChange = OnBeaconStatusChange,
    .OnSysTimeUpdate = OnSysTimeUpdate,
};

static LmHandlerParams_t LmHandlerParams =
{
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

static LmhpComplianceParams_t LmhpComplianceParams =
{
    .FwVersion.Value = FIRMWARE_VERSION,
    .OnTxPeriodicityChanged = OnTxPeriodicityChanged,
    .OnTxFrameCtrlChanged = OnTxFrameCtrlChanged,
    .OnPingSlotPeriodicityChanged = OnPingSlotPeriodicityChanged,
};

/*!
 * Defines the maximum size for the buffer receiving the fragmentation result.
 *
 * \remark By default FragDecoder.h defines:
 *         \ref FRAG_MAX_NB   21
 *         \ref FRAG_MAX_SIZE 50
 *
 *         FileSize = FRAG_MAX_NB * FRAG_MAX_SIZE
 *
 *         If bigger file size is to be received or is fragmented differently
 *         one must update those parameters.
 */
#define UNFRAGMENTED_DATA_SIZE                     ( 21 * 50 )

/*
 * Un-fragmented data storage.
 */
static uint8_t UnfragmentedData[UNFRAGMENTED_DATA_SIZE];

static LmhpFragmentationParams_t FragmentationParams =
{
#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
    .DecoderCallbacks = 
    {
        .FragDecoderWrite = FragDecoderWrite,
        .FragDecoderRead = FragDecoderRead,
    },
#else
    .Buffer = UnfragmentedData,
    .BufferSize = UNFRAGMENTED_DATA_SIZE,
#endif
    .OnProgress = OnFragProgress,
    .OnDone = OnFragDone
};

/*!
 * Indicates if LoRaMacProcess call is pending.
 * 
 * \warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static volatile uint8_t IsMacProcessPending = 0;

static volatile uint8_t IsTxFramePending = 0;

static volatile uint32_t TxPeriodicity = 0;

/*
 * Indicates if the system time has been synchronized
 */
static volatile bool IsClockSynched = false;

/*
 * MC Session Started
 */
static volatile bool IsMcSessionStarted = false;

/*
 * Indicates if the file transfer is done
 */
static volatile bool IsFileTransferDone = false;

/*
 *  Received file computed CRC32
 */
static volatile uint32_t FileRxCrc = 0;

/*!
 * UART object used for command line interface handling
 */
//extern Uart_t Uart2;

//immer auf strombegrenzung durch netzteil achten

//next goal: get the appkey into the transmission & maybe use different example: 
//afterwards look for the receive when its possible under ttn application/end devices


/*!
 * Main application entry point.
 */
void fouta( void )
{
    BoardInitMcu( );
    BoardInitPeriph( );

    TimerInit( &Led1Timer, OnLed1TimerEvent );
    TimerSetValue( &Led1Timer, 25 );

    TimerInit( &Led2Timer, OnLed2TimerEvent );
    TimerSetValue( &Led2Timer, 100 );

    TimerInit( &LedBeaconTimer, OnLedBeaconTimerEvent );
    TimerSetValue( &LedBeaconTimer, 5000 );

    // Initialize transmission periodicity variable
    TxPeriodicity = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );

    const Version_t appVersion = { .Value = FIRMWARE_VERSION };
    const Version_t gitHubVersion = { .Value = GITHUB_VERSION };
    DisplayAppInfo( "fuota-test-01", 
                    &appVersion,
                    &gitHubVersion );
//	SX126xSetRegulatorMode( USE_DCDC );
//	SX126xGetRandom();
//	uint32_t dummy = SX126xReadRegister( REG_RX_GAIN );		//it seems that the SPI communication works
//	SX126xWriteRegister(REG_RX_GAIN, dummy + 2);			//however, there is still something broken 	
//	dummy = SX126xReadRegister( REG_RX_GAIN );				//problem was power draw from the lambda board
//	SX126xWriteRegister(REG_RX_GAIN, dummy - 2);
//	SX126xSetRegulatorMode( USE_DCDC );
//	SX126xSetRegulatorMode( USE_DCDC );
//	SX126xSetRx( 0xFFFFF ); // Rx Continuous
//	SX126xGetRandom();
    if ( LmHandlerInit( &LmHandlerCallbacks, &LmHandlerParams ) != LORAMAC_HANDLER_SUCCESS )
    {
        am_util_stdio_printf( "LoRaMac wasn't properly initialized\n" );
        // Fatal error, endless loop.
        while ( 1 )
        {
        }
    }

    // Set system maximum tolerated rx error in milliseconds
    LmHandlerSetSystemMaxRxError( 20 );

    // The LoRa-Alliance Compliance protocol package should always be
    // initialized and activated.
    LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams );
    LmHandlerPackageRegister( PACKAGE_ID_CLOCK_SYNC, NULL );
    LmHandlerPackageRegister( PACKAGE_ID_REMOTE_MCAST_SETUP, NULL );
    LmHandlerPackageRegister( PACKAGE_ID_FRAGMENTATION, &FragmentationParams );

    IsClockSynched = false;
    IsFileTransferDone = false;

    LmHandlerJoin( );
//	Radio.SetTxContinuousWave(868100000ul,0,1000);
//	uint8_t buf[2];
//	RadioStatus_t status = SX126xGetStatus();
//	RadioError_t error = SX126xGetDeviceErrors();
//	SX126xReadCommand(RADIO_GET_ERROR, buf, 2);
	
	//next step is to compare the communication with the working sending
	//theres also a big voltage drop visible -> pA/ TX settings
	
    StartTxProcess( LORAMAC_HANDLER_TX_ON_TIMER );

    while( 1 )
    {
        // Process characters sent over the command line interface
        //CliProcess( &Uart2 );

        // Processes the LoRaMac events
        LmHandlerProcess( );

        // Process application uplinks management
        UplinkProcess( );

        CRITICAL_SECTION_BEGIN( );
        if( IsMacProcessPending == 1 )
        {
            // Clear flag and prevent MCU to go into low power modes.
            IsMacProcessPending = 0;
        }
        else
        {
            // The MCU wakes up through events
            BoardLowPowerHandler( );
        }
        CRITICAL_SECTION_END( );
    }
}

static void OnMacProcessNotify( void )
{
    IsMacProcessPending = 1;
}

static void OnNvmDataChange( LmHandlerNvmContextStates_t state, uint16_t size )
{
    DisplayNvmDataChange( state, size );
}

static void OnNetworkParametersChange( CommissioningParams_t* params )
{
    DisplayNetworkParametersUpdate( params );
}

static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn )
{
    DisplayMacMcpsRequestUpdate( status, mcpsReq, nextTxIn );
}

static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn )
{
    DisplayMacMlmeRequestUpdate( status, mlmeReq, nextTxIn );
}

static void OnJoinRequest( LmHandlerJoinParams_t* params )
{
    DisplayJoinRequestUpdate( params );
    if( params->Status == LORAMAC_HANDLER_ERROR )
    {
        LmHandlerJoin( );
    }
    else
    {
        LmHandlerRequestClass( LORAWAN_DEFAULT_CLASS );
    }
}

static void OnTxData( LmHandlerTxParams_t* params )
{
    DisplayTxUpdate( params );
}

static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params )
{
    DisplayRxUpdate( appData, params );
}

static void OnClassChange( DeviceClass_t deviceClass )
{
    DisplayClassUpdate( deviceClass );

    switch( deviceClass )
    {
        default:
        case CLASS_A:
        {
            IsMcSessionStarted = false;
            break;
        }
        case CLASS_B:
        {
            // Inform the server as soon as possible that the end-device has switched to ClassB
            LmHandlerAppData_t appData =
            {
                .Buffer = NULL,
                .BufferSize = 0,
                .Port = 0,
            };
            LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
            IsMcSessionStarted = true;
            break;
        }
        case CLASS_C:
        {
            IsMcSessionStarted = true;
            // Switch LED 2 ON
            gpioWrite( ADC_SENSOREXTRA2, 0 );
            break;
        }
    }
}

static void OnBeaconStatusChange( LoRaMacHandlerBeaconParams_t* params )
{
    switch( params->State )
    {
        case LORAMAC_HANDLER_BEACON_RX:
        {
            TimerStart( &LedBeaconTimer );
            break;
        }
        case LORAMAC_HANDLER_BEACON_LOST:
        case LORAMAC_HANDLER_BEACON_NRX:
        {
            TimerStop( &LedBeaconTimer );
            break;
        }
        default:
        {
            break;
        }
    }

    DisplayBeaconUpdate( params );
}

#if( LMH_SYS_TIME_UPDATE_NEW_API == 1 )
static void OnSysTimeUpdate( bool isSynchronized, int32_t timeCorrection )
{
    IsClockSynched = isSynchronized;
}
#else
static void OnSysTimeUpdate( void )
{
    IsClockSynched = true;
}
#endif

#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
static int8_t FragDecoderWrite( uint32_t addr, uint8_t *data, uint32_t size )
{
    if( size >= UNFRAGMENTED_DATA_SIZE )
    {
        return -1; // Fail
    }
    for(uint32_t i = 0; i < size; i++ )
    {
        UnfragmentedData[addr + i] = data[i];
    }
    return 0; // Success
}

static int8_t FragDecoderRead( uint32_t addr, uint8_t *data, uint32_t size )
{
    if( size >= UNFRAGMENTED_DATA_SIZE )
    {
        return -1; // Fail
    }
    for(uint32_t i = 0; i < size; i++ )
    {
        data[i] = UnfragmentedData[addr + i];
    }
    return 0; // Success
}
#endif

static void OnFragProgress( uint16_t fragCounter, uint16_t fragNb, uint8_t fragSize, uint16_t fragNbLost )
{
    // Switch LED 2 OFF for each received downlink
    gpioWrite( ADC_SENSOREXTRA2, 1 );
    TimerStart( &Led2Timer );

    am_util_stdio_printf( "\n###### =========== FRAG_DECODER ============ ######\n" );
    am_util_stdio_printf( "######               PROGRESS                ######\n");
    am_util_stdio_printf( "###### ===================================== ######\n");
    am_util_stdio_printf( "RECEIVED    : %5d / %5d Fragments\n", fragCounter, fragNb );
    am_util_stdio_printf( "              %5d / %5d Bytes\n", fragCounter * fragSize, fragNb * fragSize );
    am_util_stdio_printf( "LOST        :       %7d Fragments\n\n", fragNbLost );
}

#if( FRAG_DECODER_FILE_HANDLING_NEW_API == 1 )
static void OnFragDone( int32_t status, uint32_t size )
{
    FileRxCrc = Crc32( UnfragmentedData, size );
    IsFileTransferDone = true;
    // Switch LED 2 OFF
    gpioWrite( ADC_SENSOREXTRA2, 1 );

    am_util_stdio_printf( "\n###### =========== FRAG_DECODER ============ ######\n" );
    am_util_stdio_printf( "######               FINISHED                ######\n");
    am_util_stdio_printf( "###### ===================================== ######\n");
    am_util_stdio_printf( "STATUS      : %ld\n", status );
    am_util_stdio_printf( "CRC         : %08lX\n\n", FileRxCrc );
}
#else
static void OnFragDone( int32_t status, uint8_t *file, uint32_t size )
{
    FileRxCrc = Crc32( file, size );
    IsFileTransferDone = true;
    // Switch LED 2 OFF
    GpioWrite( &Led2, 1 );

    am_util_stdio_printf( "\n###### =========== FRAG_DECODER ============ ######\n" );
    am_util_stdio_printf( "######               FINISHED                ######\n");
    am_util_stdio_printf( "###### ===================================== ######\n");
    am_util_stdio_printf( "STATUS      : %ld\n", status );
    am_util_stdio_printf( "CRC         : %08lX\n\n", FileRxCrc );
}
#endif

static void StartTxProcess( LmHandlerTxEvents_t txEvent )
{
    switch( txEvent )
    {
    default:
        // Intentional fall through
    case LORAMAC_HANDLER_TX_ON_TIMER:
        {
            // Schedule 1st packet transmission
            TimerInit( &TxTimer, OnTxTimerEvent );
            TimerSetValue( &TxTimer, TxPeriodicity );
            OnTxTimerEvent( NULL );
        }
        break;
    case LORAMAC_HANDLER_TX_ON_EVENT:
        {
        }
        break;
    }
}

static void UplinkProcess( void )
{
    LmHandlerErrorStatus_t status = LORAMAC_HANDLER_ERROR;

    if( LmHandlerIsBusy( ) == true )
    {
        return;
    }

    uint8_t isPending = 0;
    CRITICAL_SECTION_BEGIN( );
    isPending = IsTxFramePending;
    IsTxFramePending = 0;
    CRITICAL_SECTION_END( );
    if( isPending == 1 )
    {
        if( IsMcSessionStarted == false )
        {
            if( IsFileTransferDone == false )
            {
                if( IsClockSynched == false )
                {
                    status = LmhpClockSyncAppTimeReq( );
                }
                else
                {
                    AppDataBuffer[0] = randr( 0, 255 );
                    // Send random packet
                    LmHandlerAppData_t appData =
                    {
                        .Buffer = AppDataBuffer,
                        .BufferSize = 1,
                        .Port = 1,
                    };
                    status = LmHandlerSend( &appData, LmHandlerParams.IsTxConfirmed );
                }
            }
            else
            {
                AppDataBuffer[0] = 0x05; // FragDataBlockAuthReq
                AppDataBuffer[1] = FileRxCrc & 0x000000FF;
                AppDataBuffer[2] = ( FileRxCrc >> 8 ) & 0x000000FF;
                AppDataBuffer[3] = ( FileRxCrc >> 16 ) & 0x000000FF;
                AppDataBuffer[4] = ( FileRxCrc >> 24 ) & 0x000000FF;

                // Send FragAuthReq
                LmHandlerAppData_t appData =
                {
                    .Buffer = AppDataBuffer,
                    .BufferSize = 5,
                    .Port = 201,
                };
                status = LmHandlerSend( &appData, LmHandlerParams.IsTxConfirmed );
            }
            if( status == LORAMAC_HANDLER_SUCCESS )
            {
                // Switch LED 1 ON
                gpioWrite( AS7341_INT_PIN, 0 );
                TimerStart( &Led1Timer );
            }
        }
    }
}

static void OnTxPeriodicityChanged( uint32_t periodicity )
{
    TxPeriodicity = periodicity;

    if( TxPeriodicity == 0 )
    { // Revert to application default periodicity
        TxPeriodicity = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
    }

    // Update timer periodicity
    TimerStop( &TxTimer );
    TimerSetValue( &TxTimer, TxPeriodicity );
    TimerStart( &TxTimer );
}

static void OnTxFrameCtrlChanged( LmHandlerMsgTypes_t isTxConfirmed )
{
    LmHandlerParams.IsTxConfirmed = isTxConfirmed;
}

static void OnPingSlotPeriodicityChanged( uint8_t pingSlotPeriodicity )
{
    LmHandlerParams.PingSlotPeriodicity = pingSlotPeriodicity;
}

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent( void* context )
{
    TimerStop( &TxTimer );

    IsTxFramePending = 1;

    // Schedule next transmission
    TimerSetValue( &TxTimer, TxPeriodicity );
    TimerStart( &TxTimer );
}

/*!
 * Function executed on Led 1 Timeout event
 */
static void OnLed1TimerEvent( void* context )
{
    TimerStop( &Led1Timer );
    // Switch LED 1 OFF
    gpioWrite( AS7341_INT_PIN, 1 );
}

/*!
 * Function executed on Led 2 Timeout event
 */
static void OnLed2TimerEvent( void* context )
{
    TimerStop( &Led2Timer );
    // Switch LED 2 ON
    gpioWrite( ADC_SENSOREXTRA2, 0 );
}

/*!
 * \brief Function executed on Beacon timer Timeout event
 */
static void OnLedBeaconTimerEvent( void* context )
{
    gpioWrite( ADC_SENSOREXTRA2, 0 );
    TimerStart( &Led2Timer );

    TimerStart( &LedBeaconTimer );
}