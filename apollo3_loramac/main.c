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

#include "taskHandler.h"

// === Hardware Selection ===
// 1. Go to the Project menu and choose Options for Target 'Target 1' (or press Alt+F7).
// 2. In the window that opens, go to the C/C++ tab.
// 3. Look for the field labeled Define.
// 4. Fill with the hardware you like to use. 
//		e.g. HW_CF_SENSOR for Chlorophyll Fluorescence Sensor or HW_ECOVETTE for the ECOvette sensor

// === Sanity Check ===
#if defined(HW_CF_SENSOR) && defined(HW_ECOVETTE)
#error "Only one hardware version should be defined!"
#elif!defined(HW_CF_SENSOR) && !defined(HW_ECOVETTE)
#error "You must define exactly one hardware version!"
#endif

// The processor clock is initialized by CMSIS startup + system file
int main(void) { // User application starts here
  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
  am_hal_pwrctrl_low_power_init();
  am_hal_rtc_osc_disable();
  enable_printf();
  am_util_stdio_printf("Ecosense Vegetation 2025");

  // Set the clock frequency.
  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

  // Set the default cache configuration
  am_hal_cachectrl_config( & am_hal_cachectrl_defaults);
  am_hal_cachectrl_enable();

  // Set the default cache configuration and enable it.
  if (AM_HAL_STATUS_SUCCESS != am_hal_cachectrl_config( & am_hal_cachectrl_defaults)) {
    am_util_stdio_printf("Error - configuring the system cache failed.\n");
  }

  if (AM_HAL_STATUS_SUCCESS != am_hal_cachectrl_enable()) {
    am_util_stdio_printf("Error - enabling the system cache failed.\n");
  }
	
	am_hal_gpio_pinconfig(37, g_AM_HAL_GPIO_OUTPUT); // Configure D1 to be an output pin (red led)
	am_hal_gpio_pinconfig(38, g_AM_HAL_GPIO_OUTPUT); // Configure D2 to be an output pin (green led)
	
	
  am_hal_gpio_state_write(37, AM_HAL_GPIO_OUTPUT_SET); // set red led to high
	am_hal_gpio_state_write(38, AM_HAL_GPIO_OUTPUT_CLEAR); // set green led to low

	am_util_delay_ms(500); // wait 500 ms
	
	am_hal_gpio_state_write(37, AM_HAL_GPIO_OUTPUT_CLEAR); // set red led to low
	am_hal_gpio_state_write(38, AM_HAL_GPIO_OUTPUT_SET); // set green led to high
	
	am_util_delay_ms(500); // wait 500 ms
	am_hal_gpio_state_write(38, AM_HAL_GPIO_OUTPUT_CLEAR); // set green led to low

  periodicUplink();

  while (1) { //this while(1) loop seems to break the debugger, however the actual goal was testing the RTC alarm by enabling interrupts globally
    __NOP();
  }
}