/*!
 *  \file       main.c
 *
 *  \brief      This file provides an empty C project to start with
 *
 *  \date       09.11.2023
 *
 *  \author     Timm Luhmann (IMTEK)
 */

#include "apollo3.h"		//provides access to all necessary hardware libraries
#include "am_mcu_apollo.h"	//provides access to all necessary hardware libraries
#include "am_util_stdio.h"	//required for printf operations on console
#include "arm_math.h"		//required for ARM math operations
#include "am_util_delay.h"	//required for simple delay loops
#include "boardControl.h"
#include "rtc-board.h"
#include "fuota.h"
#include "periodicUplink.h"

void isrGPIO(void);


//there are problems with the printf/ITM to SWO interface, currently unknown whats the problem

// The processor clock is initialized by CMSIS startup + system file
int main (void) { // User application starts here
	am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX,0);
	am_hal_pwrctrl_low_power_init();
	am_hal_rtc_osc_disable();
	enable_printf();
	
	uint32_t dummy1 = 0;
	uint32_t dummy2 = 0;
	uint32_t dummy3 = 0;
	uint32_t dummy4 = 0;
	am_util_stdio_printf("ich krieg die krise hier");
	
	periodicUplink();
	
	//fouta();
	
	
//	am_hal_interrupt_master_enable();
//	NVIC_EnableIRQ(GPIO_IRQn);
//	am_hal_gpio_pinconfig(LORA_IRQ,gpioLoraIRQ);
//	AM_HAL_GPIO_MASKCREATE(GpioIntMask);
//	am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, LORA_IRQ));
//	am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, LORA_IRQ));
//	am_hal_gpio_interrupt_register(LORA_IRQ,*(am_hal_gpio_handler_t)isrGPIO);
	
	
	//test the rtc implementation for lora
//	RtcInit();
//	dummy1 = RtcSetTimerContext();
//	dummy2 = RtcGetTimerContext();
//	dummy3 = RtcMs2Tick(137);
//	dummy4 = RtcTick2Ms(dummy3);
//	RtcDelayMs(1000);
//	RtcSetAlarm(500);
//	//boardTest();
	//!     am_hal_interrupt_master_enable();

	while(1){		//this while(1) loop seems to break the debugger, however the actual goal was testing the RTC alarm by enabling interrupts globally
		__NOP();
	}
}

//void isrGPIO(void){
//	am_hal_gpio_interrupt_clear(LORA_IRQ);	
//}



