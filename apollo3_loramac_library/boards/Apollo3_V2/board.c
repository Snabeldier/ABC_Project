/*!
 * \file      board.c
 *
 * \brief     Target board general functions implementation
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

#include "utilities.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"

#include "timer.h"

#include "rtc-board.h"
#include "boardControl.h"

#include "sx126x-board.h"

#include "board.h"

/*!
 * Unique Devices IDs register set ( STM32L152x )
 */
//#define         ID1                                 ( MCUCTRL->CHIPID0 )
//#define         ID2                                 ( MCUCTRL->CHIPID1 )
//#define         ID3                                 ( MCUCTRL->CHIPPN )




void BoardCriticalSectionBegin( uint32_t *mask )
{
    *mask = __get_PRIMASK( );
    __disable_irq( );
}

void BoardCriticalSectionEnd( uint32_t *mask )
{
    __set_PRIMASK( *mask );
}

void BoardInitPeriph( void )
{
	turnOnLoRa();		//enable power for the sx1262 board
	am_hal_gpio_pinconfig(ADC_SENSOREXTRA2,g_AM_HAL_GPIO_OUTPUT);			  //debugging purpose
	spi_init();			//initialize the SPI communication driver
	device_I2C_init();	//initialize the I2C communication driver
	enable_printf();
	timerDelay(500);
	am_devices_am1805_config_init();
	
	am_hal_gpio_pinconfig(ADC_POWEREXTRA,g_AM_HAL_GPIO_INPUT_PULLUP);				//to control the FRAM delete
}

void BoardInitMcu( void )
{
	am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX,0);
	am_hal_pwrctrl_low_power_init();
	am_hal_rtc_osc_disable();
	RtcInit();
}

void BoardResetMcu( void )
{
    CRITICAL_SECTION_BEGIN( );

    //Restart system
    NVIC_SystemReset( );
}

void BoardDeInitMcu( void )
{

}

uint32_t BoardGetRandomSeed( void )
{
	uint32_t ID1 = MCUCTRL->CHIPID0;
	uint32_t ID2 = MCUCTRL->CHIPID1;
	uint32_t ID3 = MCUCTRL->CHIPPN;
    return ( ( ID1 ) ^ ( ID2 ) ^ ( ID3 ) );
}

void BoardGetUniqueId( uint8_t *id )
{
	uint32_t ID1 = MCUCTRL->CHIPID0;
	uint32_t ID2 = MCUCTRL->CHIPID1;
	uint32_t ID3 = MCUCTRL->CHIPPN;
    id[7] = ( ( ID1 ) ) >> 24;
    id[6] = ( ( ID1 ) ) >> 16;
    id[5] = ( ( ID1 ) ) >> 8;
    id[4] = ( ( ID1 ) );
    id[3] = ( ( ID2 ) ) >> 24;
    id[2] = ( ( ID2 ) ) >> 16;
    id[1] = ( ( ID2 ) ) >> 8;
    id[0] = ( ( ID2 ) );
}


uint16_t BoardBatteryMeasureVoltage( void )
{
	return 0;
}

uint32_t BoardGetBatteryVoltage( void )
{
    return 0;
}

uint8_t BoardGetBatteryLevel( void )
{
	return 0;
}

int16_t BoardGetTemperature( void )
{
	return 0;
}

uint8_t GetBoardPowerSource( void )
{
	return USB_POWER;
}


uint8_t BoardGetPotiLevel( void ){
	return 0;		//not applicable for my board
}

void BoardLowPowerHandler( void )
{
    //__disable_irq( );
    /*!
     * If an interrupt has occurred after __disable_irq( ), it is kept pending 
     * and cortex will not enter low power anyway
     */

    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);

    //__enable_irq( );
}