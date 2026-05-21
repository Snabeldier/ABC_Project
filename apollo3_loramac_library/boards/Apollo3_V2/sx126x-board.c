/*!
 * \file      sx126x-board.c
 *
 * \brief     Target board SX126x driver implementation
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
 */

#ifdef __cplusplus
extern "C"
{
#endif


#include "gpio.h"
#include "sx126x-board.h"
#include "delay.h"
#include "transmitConfig.h"
#include "boardControl.h"


/*!
 * \brief Holds the internal operating mode of the radio
 */
static RadioOperatingModes_t OperatingMode;

/*!
 * \brief Initializes the radio I/Os pins interface
 */
void SX126xIoInit( void ){
	am_hal_gpio_pinconfig(LORA_BUSY,gpioLoraInput);
}

/*!
 * \brief Initializes DIO IRQ handlers
 *
 * \param [IN] irqHandlers Array containing the IRQ callback functions
 */
void SX126xIoIrqInit( DioIrqHandler dioIrq ){
	am_hal_gpio_pinconfig(LORA_IRQ,gpioLoraIRQ);
	AM_HAL_GPIO_MASKCREATE(GpioIntMask);
	am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, LORA_IRQ));
	am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, LORA_IRQ));
	am_hal_gpio_interrupt_register(LORA_IRQ,*(am_hal_gpio_handler_t)dioIrq);
	am_hal_interrupt_master_enable();
	NVIC_EnableIRQ(GPIO_IRQn);
}

/*!
 * \brief De-initializes the radio I/Os pins interface.
 *
 * \remark Useful when going in MCU low power modes
 */
void SX126xIoDeInit( void ){
	am_hal_gpio_pinconfig(LORA_BUSY, g_AM_HAL_GPIO_DISABLE);
	am_hal_gpio_pinconfig(LORA_IRQ, g_AM_HAL_GPIO_DISABLE);
}

/*!
 * \brief Initializes the TCXO power pin.
 */
void SX126xIoTcxoInit( void ){
	//no pin for tcxo
}

/*!
 * \brief Initializes RF switch control pins.
 */
void SX126xIoRfSwitchInit( void ){
	am_hal_gpio_pinconfig(LORA_TXSWITCH, g_AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_pinconfig(LORA_RXSWITCH, g_AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_pinconfig(AS7341_INT_PIN, g_AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_pinconfig(ADC_SENSOREXTRA2, g_AM_HAL_GPIO_OUTPUT);
}

/*!
 * \brief Initializes the radio debug pins.
 */
void SX126xIoDbgInit( void )
{
#if defined( USE_RADIO_DEBUG )
	am_hal_gpio_pinconfig(LORA_TXSWITCH, g_AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_pinconfig(LORA_RXSWITCH, g_AM_HAL_GPIO_OUTPUT);

#endif
}
/*!
 * \brief HW Reset of the radio
 */
void SX126xReset( void ){
	DelayMs( 10 );
    am_hal_gpio_pinconfig(LORA_RESET, g_AM_HAL_GPIO_OUTPUT);
	gpioWrite(LORA_RESET,0);
    DelayMs( 20 );
    gpioWrite(LORA_RESET,1);
    DelayMs( 10 );
}

/*!
 * \brief Blocking loop to wait while the Busy pin in high
 */
void SX126xWaitOnBusy( void ){
	DelayMs(1);
    while( gpioRead( LORA_BUSY ) == 1 );
}

/*!
 * \brief Wakes up the radio
 */
void SX126xWakeup( void ){
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

/*!
 * \brief Send a command that write data to the radio
 *
 * \param [in]  opcode        Opcode of the command
 * \param [in]  buffer        Buffer to be send to the radio
 * \param [in]  size          Size of the buffer to send
 */
void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size ){
	SX126xCheckDeviceReady( );
	
	spi_command_write(1,command,(uint32_t *) buffer,size);

    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}
/*!
 * \brief Send a command that read data from the radio
 *
 * \param [in]  opcode        Opcode of the command
 * \param [out] buffer        Buffer holding data from the radio
 * \param [in]  size          Size of the buffer
 *
 * \retval status Return command radio status
 */
uint8_t SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size ){
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

/*!
 * \brief Write a single byte of data to the radio memory
 *
 * \param [in]  address       The address of the first byte to write in the radio
 * \param [in]  value         The data to be written in radio's memory
 */
void SX126xWriteRegister( uint16_t address, uint8_t value ){
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

/*!
 * \brief Read a single byte of data from the radio memory
 *
 * \param [in]  address       The address of the first byte to write in the radio
 *
 * \retval      value         The value of the byte at the given address in radio's memory
 */
uint8_t SX126xReadRegister( uint16_t address ){
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
	uint32_t data = offset<<8;
	data += RADIO_READ_BUFFER<<16;
	//spi_command_write(1,RADIO_READ_BUFFER, (uint32_t*)&offset ,1);	//need to check if this works in one command
	spi_command_read(3,data,(uint32_t*) buffer, size);

    SX126xWaitOnBusy( );
}

/*!
 * \brief Sets the radio output power.
 *
 * \param [IN] power Sets the RF output power
 */
void SX126xSetRfTxPower( int8_t power ){
	SX126xSetTxParams( power, RADIO_RAMP_40_US ); //40
}

/*!
 * \brief Gets the device ID
 *
 * \retval id Connected device ID
 */
uint8_t SX126xGetDeviceId( void ){
	#ifdef SX1261_CHIP
    return SX1261;
#else
    return SX1262;
#endif
}

/*!
 * \brief Initializes the RF Switch I/Os pins interface
 */
void SX126xAntSwOn( void ){
#ifdef ANT_SW_PIN
	gpioWrite(ANT_SW_PIN,1);
#endif
}

/*!
 * \brief De-initializes the RF Switch I/Os pins interface
 *
 * \remark Needed to decrease the power consumption in MCU low power modes
 */
void SX126xAntSwOff( void ){
#ifdef ANT_SW_PIN
	gpioWrite(ANT_SW_PIN,0);	
#endif	
}


void SX126xRXSwOn( void ){
#ifdef RX_SW_PIN
    // Special case for Lambda board, see datasheet LAMBDA62 p.3
	gpioWrite(RX_SW_PIN,1);
#endif
}

void SX126xRXSwOff( void ){
#ifdef RX_SW_PIN
    // Special case for Lambda board, see datasheet LAMBDA62 p.3
	gpioWrite(RX_SW_PIN,0);
#endif
}
/*!
 * \brief Checks if the given RF frequency is supported by the hardware
 *
 * \param [IN] frequency RF frequency to be checked
 * \retval isSupported [true: supported, false: unsupported]
 */
bool SX126xCheckRfFrequency( uint32_t frequency ){
    // Implement check. Currently all frequencies are supported
    return true;	
}

/*!
 * \brief Gets the Defines the time required for the TCXO to wakeup [ms].
 *
 * \retval time Board TCXO wakeup time in ms.
 */
uint32_t SX126xGetBoardTcxoWakeupTime( void )
{
    return BOARD_TCXO_WAKEUP_TIME;
}

/*!
 * \brief Gets current state of DIO1 pin state.
 *
 * \retval state DIO1 pin current state.
 */
uint32_t SX126xGetDio1PinState( void ){
	return gpioRead(LORA_IRQ);
}

/*!
 * \brief Gets the current Radio OperationMode variable
 *
 * \retval      RadioOperatingModes_t last operating mode
 */
RadioOperatingModes_t SX126xGetOperatingMode( void ){
	return OperatingMode;
}

/*!
 * \brief Sets/Updates the current Radio OperationMode variable.
 *
 * \remark WARNING: This function is only required to reflect the current radio
 *                  operating mode when processing interrupts.
 *
 * \param [in] mode           New operating mode
 */
void SX126xSetOperatingMode( RadioOperatingModes_t mode ){
	OperatingMode = mode;
	switch( mode )
    {
        case MODE_TX:
            SX126xRXSwOn(  );
			gpioWrite(AS7341_INT_PIN,0);
            SX126xAntSwOff(  );
			gpioWrite(ADC_SENSOREXTRA2,1);
            break;
        case MODE_RX:
            SX126xRXSwOff(  );
			gpioWrite(AS7341_INT_PIN,1);
            SX126xAntSwOn(  );
			gpioWrite(ADC_SENSOREXTRA2,0);
            break;
        case MODE_RX_DC:
            SX126xRXSwOff(  );
			gpioWrite(AS7341_INT_PIN,1);
            SX126xAntSwOn(  );
			gpioWrite(ADC_SENSOREXTRA2,0);
            break;
        default:
            SX126xRXSwOff(  );
			gpioWrite(AS7341_INT_PIN,0);
            SX126xAntSwOn(  );
			gpioWrite(ADC_SENSOREXTRA2,0);
            break;
    }
}

/*!
 * Radio hardware and global parameters
 */
extern SX126x_t SX126x;

#ifdef __cplusplus
}
#endif

LoRaMacStatus_t LoRaMacSendFrame(uint8_t *FRMPayload, uint8_t FRMPayloadLen,
                                 uint8_t *FOpts, uint8_t FOptsLen);

void LoRaMacComputeMic( const uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint32_t *mic );

void LoRaMacPayloadEncrypt( uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint8_t *encBuffer );
