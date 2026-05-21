/*!
 *  \file       timing.c
 *
 *  \brief      Provides functions to configure the timer
 *
 *  \date       01.03.2023
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
//***** Header Files **********************************************************
//#include <stdint.h>
//#include <stdbool.h>
#include "math.h"
#include "am_mcu_apollo.h"
#include "pins.h"
#include "timing.h"
#include "device.h"

#include "am_util_stdio.h"

//***** Defines ***************************************************************

#define WAKE_INTERVAL_IN_MS 1
#define XT_PERIOD 32768
#define HFRC_PERIOD 3000000
//#define WAKE_INTERVAL XT_PERIOD * WAKE_INTERVAL_IN_MS * 1e-3

//#define WAKE_INTERVAL_IN_US 350
//#define WAKE_INTERVAL XT_PERIOD * WAKE_INTERVAL_IN_US * 1e-6

//***** Functions *************************************************************
int pin = 0;
int wait = 0;
extern uint8_t PAM_phase;
uint32_t interval;
uint16_t dacdata = 0xFFA0;	//if not other defined generate around 800mV

volatile bool g_delayDone = false;  // Flag set in the CTIMER ISR when the delay expires

/*
* 
* @brief Function to initialize the system timer
*		 Currently using 3 MHz clock
*
*/
void myTimerInit(uint8_t state){
	interval = 16000;
	am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA);
	am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREF);
	am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
	NVIC_SetPriority(ADC_IRQn,1);	//set lower priority for adc
	NVIC_SetPriority(STIMER_CMPR0_IRQn,0);	//set the highest priority level
	NVIC_EnableIRQ(STIMER_CMPR0_IRQn);
	NVIC_EnableIRQ(STIMER_CMPR5_IRQn);
	am_hal_stimer_compare_delta_set(0, interval);
	if(state == 0){
		am_hal_stimer_compare_delta_set(5,600000);
	}
	else if(state == 1){
		am_hal_stimer_compare_delta_set(5,1800000);
	}
	else{
		am_hal_stimer_compare_delta_set(5,3000000);
	}
	////am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);
	am_hal_stimer_config(AM_HAL_STIMER_HFRC_3MHZ | AM_HAL_STIMER_CFG_COMPARE_A_ENABLE | AM_HAL_STIMER_CFG_COMPARE_F_ENABLE);
	//am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_A_ENABLE | AM_HAL_STIMER_CFG_COMPARE_F_ENABLE);
	PAM_phase = 1;	//PAM started
	pin = 1;
}

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

void myTimerDeInit(){
	am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR |AM_HAL_STIMER_CFG_FREEZE);
	am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREA);
	am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREF);
	NVIC_DisableIRQ(STIMER_CMPR0_IRQn);
	NVIC_DisableIRQ(STIMER_CMPR5_IRQn);
}


//  Configure CTIMER0 Timer A once for one-shot mode at 32kHz xtal. 
void delay_init_xtal_one_shot(void)
{
    // Make sure the CTIMER is stopped & cleared if it was in use
    am_hal_ctimer_stop(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);

    //
    //   Configure Timer 0, Segment A.
    //    - AM_HAL_CTIMER_FN_ONCE: one-shot mode
    //    - AM_HAL_CTIMER_XT_32_768KHZ: 32 kHz clock source
    //    - AM_HAL_CTIMER_INT_ENABLE: we want an interrupt on timer expiration
    //
    uint32_t cfg = (AM_HAL_CTIMER_FN_ONCE
                  | AM_HAL_CTIMER_XT_32_768KHZ
                  | AM_HAL_CTIMER_INT_ENABLE);

    am_hal_ctimer_config_single(0, AM_HAL_CTIMER_TIMERA, cfg);
}

//---------------------------------------------------------------------------
// CTIMER Delay Function Using 32.768 kHz Crystal
//---------------------------------------------------------------------------
void my_delay_ms_xtal(uint32_t ms)
{
    //
    // Each tick of the 32.768 kHz crystal = 1 / 32768 s ?? 30.5 |¨¬s
    // For 1 ms, that's about 32.768 ticks.
    // We'll do a 64-bit multiply to avoid overflow on large ms.
    //
    uint64_t ticks = ((uint64_t)ms * 32768ULL) / 1000ULL;
    if (ticks == 0)
    {
        // Ensure at least 1 tick if ms was very small
        ticks = 1;
    }

    //
    // 1) Stop & Clear CTIMER0 Timer A (in case it was running before).
    //
    am_hal_ctimer_stop(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);

    //
    // 2) Set the period to (ticks - 1). Timer counts 0..(ticks-1).
    //    No intermediate compare needed, so second param is 0.
    //
    am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, (uint32_t)(ticks - 1), 0);

    //
    // 3) Clear & enable the specific interrupt for TimerA0C0 (timer 0, segment A, compare 0).
    //
    //    By default, the 'period_set' config triggers the "TimerA0C0" interrupt
    //    when it hits (ticks - 1). Then the timer stops because it's one-shot.
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0C0);
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0C0);

    //
    // 4) Reset the global flag, then start the timer.
    //
    g_delayDone = false;
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);

    //
    // 5) Wait until the ISR sets g_delayDone = true.
    //    You can optionally call "am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP)" in
    //    the loop for lower power, but a simple while loop is shown here for clarity.
    //
    while (!g_delayDone)
    {
        // Optionally enter deep sleep to save power:
        // am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }

    //
    // Delay complete! Return to caller.
    // The timer auto-stopped in one-shot mode.
    //
}

void my_delay_us_xtal(uint32_t us)
{
    // 1) Calculate how many ticks needed
    //    We'll do rounding by adding half the divisor (500000) before dividing.
    uint64_t ticks = ((uint64_t)us * 32768ULL + 500000ULL) / 1000000ULL;
    if (ticks == 0)
    {
        ticks = 1; // at least 1 tick => ~30.5 us
    }

    // 2) Stop & clear the timer in case it was running
    am_hal_ctimer_stop(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);

    // 3) Set the period (counts from 0..(ticks-1)) 
    am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, (uint32_t)(ticks - 1), 0);

    // 4) Clear and enable the compare interrupt
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0C0);
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0C0);

    // 5) Reset the "done" flag
    g_delayDone = false;

    // 6) Start the timer
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);

    // 7) Wait until ISR signals completion
    while (!g_delayDone)
    {
        // optionally sleep:
        // am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}



/*
*
* @brief system timer 0 interrupt routine: depending on the previous state the DAC is called to switch between high and low value
*
*/
void am_stimer_cmpr0_isr(){
	//
    // Check/clear the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_compare_delta_set(0, interval);
	if(pin){
		pin = 0;
		am_devices_dac63002_1_set_output(0x0000);
		//am_util_stdio_printf("Turn off");
	}
	else{
		pin = 1;
		am_devices_dac63002_1_set_output(dacdata);
		//am_util_stdio_printf("Turn on");
	}
	
}


void am_stimer_cmpr4_isr(){
	am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREE);
	wait = 1;
}

void am_stimer_cmpr5_isr(){
	//
    // Check/clear the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREF);
	NVIC_DisableIRQ(STIMER_CMPR5_IRQn);	// disable interrupt till it gets reconfigured
	NVIC_DisableIRQ(STIMER_CMPR0_IRQn);
	am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREA);
	am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREF);
	am_devices_dac63002_1_set_output(0x0000);
	dacdata = 0;
	PAM_phase = 2;
}

void am_ctimer_isr(){
	
    // Read the interrupt status
    uint32_t ui32Status = am_hal_ctimer_int_status_get(true);

    // check if TimerA0C0 triggered:
    //

    // If it's our TimerA0C0 event:
    if (ui32Status & AM_HAL_CTIMER_INT_TIMERA0C0)
    {
        // Clear the interrupt
        am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0C0);

        // Signal that the delay has ended
        g_delayDone = true;
    }
		
		else
		{				
				am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA1);
				wait = 1;
		}
	

}

/*
* @param frequency: The frequency for the pwm generation using the external DAC in combination with the system timer
*
* @brief Function to set the timer interval to meet the desired frequency on the pwm output
*/
void setTimerFreq(uint32_t frequency){
	
	//interval = (uint32_t)round((32678/(frequency*2)));	//currently for 50% duty cycle
	interval = (uint32_t)round((3000000/(frequency*2)));	//currently for 50% duty cycle
	//am_hal_stimer_config(AM_HAL_STIMER_CFG_FREEZE);
	//am_hal_stimer_compare_delta_set(0,interval);
	//am_hal_stimer_config(AM_HAL_STIMER_CFG_THAW);
	interval -= 22;	//hand calibration for 1000 Hz currently
}

void setTimerVoltageOut(float voltage){
	dacdata = (uint16_t) round((voltage*4096)/(1.8));
	dacdata = dacdata<<4;
}