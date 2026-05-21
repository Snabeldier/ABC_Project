/*!
 *  \file       timing.c
 *
 *  \brief      Provides functions to configure the timer
 *
 *  \date       01.03.2023
 *
 *  \author     Timm Luhmann (IMTEK)
 */
//***** Header Files **********************************************************
//#include <stdint.h>
//#include <stdbool.h>
#include "math.h"
#include "am_mcu_apollo.h"
#include "pins.h"
#include "timing.h"
#include "boardControl.h"

#include "am_util_stdio.h"

//***** Defines ***************************************************************

#define WAKE_INTERVAL_IN_MS 1
#define XT_PERIOD 32768
#define HFRC_PERIOD 3000000
//#define WAKE_INTERVAL XT_PERIOD * WAKE_INTERVAL_IN_MS * 1e-3

//#define WAKE_INTERVAL_IN_US 350
//#define WAKE_INTERVAL XT_PERIOD * WAKE_INTERVAL_IN_US * 1e-6

//***** Functions *************************************************************
int wait = 0;



void timerDelay(uint32_t ms){
	wait = 0;
	uint32_t timerval = ms*3000;		//1ms equals 3000 timer ticks
	//uint32_t timerval = ms*32;		//1ms equals 32 timer ticks
	//am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA1);
	am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREE);
	am_hal_stimer_config(AM_HAL_STIMER_CFG_FREEZE);
	//am_hal_ctimer_config_single(1,AM_HAL_CTIMER_BOTH,AM_HAL_CTIMER_FN_ONCE | AM_HAL_CTIMER_HFRC_3MHZ);
	//NVIC_EnableIRQ(CTIMER_IRQn);
	NVIC_EnableIRQ(STIMER_CMPR4_IRQn);
	am_hal_stimer_compare_delta_set(4,timerval);
	//am_hal_ctimer_compare_set(1,AM_HAL_CTIMER_BOTH,0,timerval);
//	am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_E_ENABLE);
	am_hal_stimer_config(AM_HAL_STIMER_HFRC_3MHZ | AM_HAL_STIMER_CFG_COMPARE_E_ENABLE);
	//am_hal_ctimer_start(1,AM_HAL_CTIMER_BOTH);
	while(wait == 0){
		am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_NORMALSLEEP);
	}
	am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREE);
	NVIC_DisableIRQ(STIMER_CMPR4_IRQn);	
	//NVIC_DisableIRQ(CTIMER_IRQn);
}	

void am_stimer_cmpr4_isr(){
	am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREE);
	wait = 1;
}