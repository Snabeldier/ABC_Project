/*!
 * \file      sx1262x-board.c
 *
 * \brief     Target board: Uni Freiburg IMTEK Ecosense Microcontroller Platform with Ambiq Apollo 3
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
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Timm Luhmann (Uni Freiburg IMTEK), 2023
 */
 
#include "sx126x.h"
#include "am_mcu_apollo.h"
#include "spi.h"
#include "gpio.h"
#include "sx126x-board.h"
#include "pins.h"
#include "transmitConfig.h"
#include "delay.h"
#include "utilities.h"
#include "LoRaMac.h"
#include "LoRaMacCrypto.h"
#include "se-identity.h"
#include "cmac.h"
#include "aes.h"


const uint32_t device_address = LORAWAN_DEVICE_ADDRESS;
const uint8_t net_key[] = {0xDA, 0x7C, 0x78, 0x25, 0x98, 0x2F, 0x45, 0x8F, 0xB9, 0xCB, 0x6E, 0xD8, 0x9A, 0xB0, 0x16, 0xF0};
const uint8_t app_key[] = {0x9C, 0xB1, 0xF2, 0xC6, 0x88, 0xFA, 0x82, 0x36, 0xCE, 0xA4, 0xEA, 0xED, 0x4A, 0x3F, 0xD7, 0x42};

/*!
 * MIC field computation initial data
 */
static uint8_t MicBlockB0[] = { 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                              };

/*!
 * Contains the computed MIC field.
 *
 * \remark Only the 4 first bytes are used
 */
static uint8_t Mic[16];

/*!
 * Encryption aBlock and sBlock
 */
static uint8_t aBlock[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                          };
static uint8_t sBlock[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                          };
							  
/*!
 * \brief Holds the internal operating mode of the radio
 */
static RadioOperatingModes_t OperatingMode;


void SX126xIoInit( void )
{
	gpioInitLora();
    //GpioInit( &SX126x.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
    //GpioInit( &SX126x.BUSY, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    //GpioInit( &SX126x.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    //GpioInit( &DeviceSel, RADIO_DEVICE_SEL, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void SX126xIoIrqInit( DioIrqHandler dioIrq )
{
//    GpioSetInterrupt( &SX126x.DIO1, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, dioIrq );

    // Init IRQ interrupt for DIO1
//          // IRQ pin
//    gpioSetLoraIRQ(LORA_IRQ);
//    gpioInterruptClear(LORA_IRQ);
//    gpioSetInterrupt(LORA_IRQ);

//    am_hal_gpio_interrupt_register(LORA_IRQ,(am_hal_gpio_handler_t)dioIrq);
	am_hal_gpio_pinconfig(LORA_IRQ,gpioLoraIRQ);
	AM_HAL_GPIO_MASKCREATE(GpioIntMask);
	am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, LORA_IRQ));
	am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, LORA_IRQ));
	am_hal_gpio_interrupt_register(LORA_IRQ,*(am_hal_gpio_handler_t)dioIrq);
	am_hal_interrupt_master_enable();
	NVIC_EnableIRQ(GPIO_IRQn);
}

void SX126xIoDeInit( void )
{
	am_hal_gpio_pinconfig(LORA_BUSY, g_AM_HAL_GPIO_DISABLE);
	am_hal_gpio_pinconfig(LORA_IRQ, g_AM_HAL_GPIO_DISABLE);
}

void SX126xIoTcxoInit( void ){
	//no pin for tcxo
}

void SX126xIoRfSwitchInit( void ){
	
	am_hal_gpio_pinconfig(LORA_TXSWITCH, g_AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_pinconfig(LORA_RXSWITCH, g_AM_HAL_GPIO_OUTPUT);
}

void SX126xIoDbgInit( void )
{
#if defined( USE_RADIO_DEBUG )
    GpioInit( &DbgPinTx, RADIO_DBG_PIN_TX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPinRx, RADIO_DBG_PIN_RX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	am_hal_gpio_pinconfig(LORA_TXSWITCH, g_AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_pinconfig(LORA_RXSWITCH, g_AM_HAL_GPIO_OUTPUT);

#endif
}

uint32_t SX126xGetBoardTcxoWakeupTime( void )
{
    return BOARD_TCXO_WAKEUP_TIME;
}

RadioOperatingModes_t SX126xGetOperatingMode( void )
{
    return OperatingMode;
}

void SX126xSetOperatingMode( RadioOperatingModes_t mode )
{
    OperatingMode = mode;
//    SX126xAntSwOn(  );
    switch( mode )
    {
        case MODE_TX:
            SX126xRXSwOn(  );
            SX126xAntSwOff(  );
            break;
        case MODE_RX:
            SX126xRXSwOff(  );
            SX126xAntSwOn(  );
            break;
        case MODE_RX_DC:
            SX126xRXSwOff(  );
            SX126xAntSwOn(  );
            break;
        default:
            SX126xRXSwOff(  );
            SX126xAntSwOn(  );
            break;
    }
	
    //waitInLPMzero(10);
}

void SX126xReset( void )
{
    DelayMs( 10 );
    am_hal_gpio_pinconfig(LORA_RESET, g_AM_HAL_GPIO_OUTPUT);
	gpioWrite(LORA_RESET,0);
    DelayMs( 20 );
    gpioWrite(LORA_RESET,1);
    DelayMs( 10 );
}


void SX126xWaitOnBusy( void )
{
	DelayMs(1);
    while( gpioRead( LORA_BUSY ) == 1 );
}


void SX126xWakeup( void )
{
    CRITICAL_SECTION_BEGIN( );
	
	uint32_t data = 0x00;
	spi_command_write(1,RADIO_GET_STATUS, &data, 1);

	//spi_command_read(0,0,&data,2);
    // Wait for chip to be ready.
    SX126xWaitOnBusy( );

    // Update operating mode context variable
    SX126xSetOperatingMode( MODE_STDBY_RC );

    CRITICAL_SECTION_END( );
}


void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );
	
	spi_command_write(1,command,(uint32_t *) buffer,size);

    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}


uint8_t SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    uint8_t status = 0;
	int i = 0;
    SX126xCheckDeviceReady( );

	uint8_t buf[32] = {0};
	uint8_t * p_buf = buf;
	//status = spi_command_read(1,command,(uint32_t * ) buffer, size+1);
	status = spi_command_read(1,command,(uint32_t *) p_buf, size+1);
	
	for(i = 0; i<size ; i++){
		*buffer++ = buf[i+1];
	}

    SX126xWaitOnBusy( );

    return status;
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );
	uint32_t data = 0x00;
	data += address;
	data += RADIO_WRITE_REGISTER<<16;
	//spi_command_write(1,RADIO_WRITE_REGISTER,&data ,0);
	spi_command_write(3,data , (uint32_t*) buffer ,size);


    SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );
	uint8_t bufferplusone[64];
	uint32_t data = 0;
	data += address;
	data += RADIO_READ_REGISTER<<16;
	//spi_command_write(1,RADIO_READ_REGISTER,&data ,0);
	//spi_command_write(2,address,&data,1);
	spi_command_read(3,data , (uint32_t*) bufferplusone ,size+1);
	for(data = 0; data < size ; data++){
		buffer[data] = bufferplusone[data+1];
	}
    SX126xWaitOnBusy( );
}

uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t data;
    SX126xReadRegisters( address, &data, 1 );
    return data;
}


void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );
	uint32_t data = 0x00;
	data += RADIO_WRITE_BUFFER<<8;
//	uint8_t buf[128] = {0};
//	uint8_t * p_buf = buf;
//	p_buf++;
//	p_buf++;
//	p_buf = buffer;
//	*p_buf-- = 0;
//	p_buf--;

	//spi_command_write(1,RADIO_WRITE_BUFFER,&data ,0);
	//spi_command_write(1,offset,&data,0);
	//spi_command_write(0,0,(uint32_t*) buffer, size);
	spi_command_write(2,data,(uint32_t*) buffer, size);

    SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );
	uint32_t data = offset;
	data += RADIO_READ_BUFFER<<8;
	//spi_command_write(1,RADIO_READ_BUFFER, (uint32_t*)&offset ,1);	//need to check if this works in one command
	spi_command_read(2,data,(uint32_t*) buffer, size);

    SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_10_US ); //40
}

uint8_t SX126xGetDeviceId( void )
{
    //if( GpioRead( &DeviceSel ) == 1 )
    //{
    //    return SX1261;
    //}
    //else
    //{
    //    return SX1262;
    //}

#ifdef SX1261_CHIP
    return SX1261;
#else
    return SX1262;
#endif
}

/*
 * TX on
 */
void SX126xAntSwOn( void )
{
#ifdef ANT_SW_PIN
	gpioWrite(ANT_SW_PIN,1);
#endif
}

/*
 * TX off
 */
void SX126xAntSwOff( void )
{
#ifdef ANT_SW_PIN
	gpioWrite(ANT_SW_PIN,0);	
#endif
}

/*
 * RX off
 */
void SX126xRXSwOff( void )
{
#ifdef RX_SW_PIN
    // Special case for Lambda board, see datasheet LAMBDA62 p.3
	gpioWrite(RX_SW_PIN,0);
#endif
}

/*
 * RX on
 */
void SX126xRXSwOn( void )
{
#ifdef RX_SW_PIN
    // Special case for Lambda board, see datasheet LAMBDA62 p.3
	gpioWrite(RX_SW_PIN,1);
#endif
}

bool SX126xCheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

uint32_t SX126xGetDio1PinState( void )
{
    return gpioRead(LORA_IRQ);
}

#if defined( USE_RADIO_DEBUG )
void SX126xDbgPinTxWrite( uint8_t state )
{
    GpioWrite( &DbgPinTx, state );
}

void SX126xDbgPinRxWrite( uint8_t state )
{
    GpioWrite( &DbgPinRx, state );
}
#endif


static uint8_t sended = 0;

LoRaMacStatus_t LoRaMacSendFrame(uint8_t *FRMPayload, uint8_t FRMPayloadLen,
                                 uint8_t *FOpts, uint8_t FOptsLen)
{
	uint16_t messageLen = MHDR_LEN + DEVADDR_LEN + FCTRL_LEN + FCNT_LEN + FOptsLen + FPORT_LEN + FRMPayloadLen + MIC_LEN;
	if(messageLen > LORAMAC_PHY_MAXPAYLOAD)
    {
        return LORAMAC_STATUS_LENGTH_ERROR;
    }
	
	uint8_t sendData[LORAMAC_PHY_MAXPAYLOAD] = { 0 };
    uint16_t i = 0;
    sendData[i++] = 0x40;   //For TTN: Unconfirmed Data Upload

    sendData[i++] = (uint8_t) device_address;          //LSB
    sendData[i++] = (uint8_t) (device_address >> 8);
    sendData[i++] = (uint8_t) (device_address >> 16);
    sendData[i++] = (uint8_t) (device_address >> 24);    //MSB

    sendData[i++] = FOptsLen & 0x0F; //Frame Control: No Adaptive Data Rate, NO Ack, no frame pending, FOptsLen FOpts included;

    uint16_t loraFramecounter = 1868;

    sendData[i++] = (uint8_t) loraFramecounter;
    sendData[i++] = (uint8_t) (loraFramecounter >> 8);

	uint8_t j;
    if (FOptsLen > 0)
    {

        for (j = 0; j < FOptsLen; j++)
        {
            sendData[i++] = FOpts[j];
        }
    }

    sendData[i++] = LORA_PORT;

    LoRaMacPayloadEncrypt(FRMPayload, FRMPayloadLen, app_key, device_address,
    DIR_UPLINK,
                          loraFramecounter, FRMPayload);

    if (FRMPayloadLen > 0)
    {
        for (j = 0; j < FRMPayloadLen; j++)
        {
            sendData[i++] = FRMPayload[j];
        }
    }

    //Bug: seems like the conversion to uint_32 only allows
    uint32_t mic = 0xFF;
    LoRaMacComputeMic(sendData, (messageLen - MIC_LEN), net_key, device_address,
                      DIR_UPLINK,
                      loraFramecounter, &mic);

    sendData[i++] = (uint8_t) mic;
    sendData[i++] = (uint8_t) (mic >> 8);
    sendData[i++] = (uint8_t) (mic >> 16);
    sendData[i++] = (uint8_t) (mic >> 24);

    if(i > messageLen)
    {
        return LORAMAC_STATUS_LENGTH_ERROR;
    }

    Radio.Standby( );

    SX126xWaitOnBusy();

    Radio.SetChannel( LORA_RF_FREQUENCY );
    SX126xWaitOnBusy();
    SX126xSetBufferBaseAddress( 0x00, 0x00 );

    Radio.SetTxConfig(LORA_MODEM, LORA_TX_OUTPUT_POWER, LORA_FDEV,
                      LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH_TX,
                      LORA_FIX_LENGTH,
                      LORA_CRC_TX,
                      LORA_FREQ_HOP, LORA_HOP_PERIOD, LORA_IQ_INVERSION_TX,
                      LORA_TX_TIMEOUT_VALUE);

    SX126xWaitOnBusy();

    RadioStatus_t state = SX126xGetStatus();

    SX126xWaitOnBusy();
	
	RadioError_t stateerror;
    stateerror = SX126xGetDeviceErrors(  );
    state = SX126xGetStatus();

//	if(sended == 0) {
//		sended +=1;
////		Radio.Send(sendData, messageLen);
////		stateerror = SX126xGetDeviceErrors(  );
////		state = SX126xGetStatus();
////		Radio.SetChannel( LORA_RF_FREQUENCY );
////		SX126xWaitOnBusy();
//		//SX126xSetBufferBaseAddress( 0x00, 0x00 );

////		Radio.SetTxConfig(LORA_MODEM, LORA_TX_OUTPUT_POWER, LORA_FDEV,
////                      LORA_BANDWIDTH,
////                      LORA_SPREADING_FACTOR,
////                      LORA_CODINGRATE,
////                      LORA_PREAMBLE_LENGTH_TX,
////                      LORA_FIX_LENGTH,
////                      LORA_CRC_TX,
////                      LORA_FREQ_HOP, LORA_HOP_PERIOD, LORA_IQ_INVERSION_TX,
////                      LORA_TX_TIMEOUT_VALUE);

////		SX126xWaitOnBusy();
//		Radio.Send(sendData, messageLen);
//	}
//	else{
		Radio.Send(sendData, messageLen);	
//	}
	
    SX126xWaitOnBusy();

    stateerror = SX126xGetDeviceErrors(  );
    state = SX126xGetStatus();
	
	if (((uint8_t) state.Fields.CmdStatus) != 0x01)
    {
        return LORAMAC_STATUS_ERROR;
    }
    else
    {
        return LORAMAC_STATUS_OK;
    }
}

/*!
 * CMAC computation context variable
 */
static AES_CMAC_CTX AesCmacCtx[1];

/*!
 * \brief Computes the LoRaMAC frame MIC field  
 *
 * \param [IN]  buffer          Data buffer
 * \param [IN]  size            Data buffer size
 * \param [IN]  key             AES key to be used
 * \param [IN]  address         Frame address
 * \param [IN]  dir             Frame direction [0: uplink, 1: downlink]
 * \param [IN]  sequenceCounter Frame sequence counter
 * \param [OUT] mic Computed MIC field
 */
void LoRaMacComputeMic( const uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint32_t *mic )
{
    MicBlockB0[5] = dir;
    
    MicBlockB0[6] = ( address ) & 0xFF;
    MicBlockB0[7] = ( address >> 8 ) & 0xFF;
    MicBlockB0[8] = ( address >> 16 ) & 0xFF;
    MicBlockB0[9] = ( address >> 24 ) & 0xFF;

    MicBlockB0[10] = ( sequenceCounter ) & 0xFF;
    MicBlockB0[11] = ( sequenceCounter >> 8 ) & 0xFF;
    MicBlockB0[12] = ( sequenceCounter >> 16 ) & 0xFF;
    MicBlockB0[13] = ( sequenceCounter >> 24 ) & 0xFF;

    MicBlockB0[15] = size & 0xFF;

    AES_CMAC_Init( AesCmacCtx );

    AES_CMAC_SetKey( AesCmacCtx, key );

    AES_CMAC_Update( AesCmacCtx, MicBlockB0, LORAMAC_MIC_BLOCK_B0_SIZE );
    
    AES_CMAC_Update( AesCmacCtx, buffer, size & 0xFF );
    
    AES_CMAC_Final( Mic, AesCmacCtx );
    
    *mic = ( uint32_t )( ( uint32_t )Mic[3] << 24 | ( uint32_t )Mic[2] << 16 | ( uint32_t )Mic[1] << 8 | ( uint32_t )Mic[0] );
}

/*!
 * AES computation context variable
 */
static aes_context AesContext;


void LoRaMacPayloadEncrypt( uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint8_t *encBuffer )
{
    uint16_t i;
    uint8_t bufferIndex = 0;
    uint16_t ctr = 1;

    memset1( AesContext.ksch, '\0', 240 );
    aes_set_key( key, 16, &AesContext );

    aBlock[0] = 0x01;

    aBlock[5] = dir;

    aBlock[6] = ( address ) & 0xFF;
    aBlock[7] = ( address >> 8 ) & 0xFF;
    aBlock[8] = ( address >> 16 ) & 0xFF;
    aBlock[9] = ( address >> 24 ) & 0xFF;

    aBlock[10] = ( sequenceCounter ) & 0xFF;
    aBlock[11] = ( sequenceCounter >> 8 ) & 0xFF;
    aBlock[12] = ( sequenceCounter >> 16 ) & 0xFF;
    aBlock[13] = ( sequenceCounter >> 24 ) & 0xFF;

    while( size >= 16 )
    {
        aBlock[15] = ( ( ctr ) & 0xFF );
        ctr++;

        aes_encrypt( aBlock, sBlock, &AesContext );

        for( i = 0; i < 16; i++ )
        {
            //encBuffer[bufferIndex + i] = buffer[bufferIndex + i] ^ sBlock[i];
            buffer[bufferIndex + i] = buffer[bufferIndex + i] ^ sBlock[i];
        }
        size -= 16;
        bufferIndex += 16;
    }

    if( size > 0 )
    {
        aBlock[15] = ( ( ctr ) & 0xFF );
        aes_encrypt( aBlock, sBlock, &AesContext );
        for( i = 0; i < size; i++ )
        {
            //encBuffer[bufferIndex + i] = buffer[bufferIndex + i] ^ sBlock[i];
            buffer[bufferIndex + i] = buffer[bufferIndex + i] ^ sBlock[i];
        }
    }
}