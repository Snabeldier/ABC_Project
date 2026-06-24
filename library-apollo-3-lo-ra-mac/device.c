/*!
 *  \file       device.c
 *
 *  \brief      Provides various functions to control external devices on the board
 *
 *  \date       15.02.2023
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
 
 
 #include "device.h"
 #include "am_util_delay.h"
 
 
 //***** Header Files **********************************************************
 
#define ARM_MATH_CM4
 
//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
 
uint32_t adcCurrentVal = 0;	//the following ones are all used for PAM measurement

float32_t adc_values[LEN];
float32_t fft_done[LEN];
float32_t avg = 0;
float32_t avgg = 0;
float32_t diffAvg = 0;

int counter_led = 0;
int counter_adc = 0;
uint32_t count = 0;

arm_rfft_fast_instance_f32 fft;
int avg_fft = 0;
int avg_fft_len = 2;
int avg_fft_cnt = 0;

//*****************************************************************************
//
//! @brief Turn on the supply for FRAM & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnFram(void){
	uint32_t success = 0;
	success += am_hal_gpio_pinconfig(ON_OFF_FRAM,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch FRAM on/off
	success += gpioWrite(ON_OFF_FRAM,true);
	return success;
}

//*****************************************************************************
//
//! @brief Turn the supply for FRAM off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffFram(void){
	uint32_t success = 0;
	success += gpioWrite(ON_OFF_FRAM,false);
	return success;
}

//*****************************************************************************
//
//! @brief Turn on the supply for I2C pull ups & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnI2C(void){
	uint32_t success = 0;
	success += am_hal_gpio_pinconfig(ON_OFF_I2C,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch I2C on/off
	success += gpioWrite(ON_OFF_I2C,true);
	success += am_hal_gpio_pinconfig(2,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch I2C on/off
	success += gpioWrite(2,true);
	return success;
}
	
//*****************************************************************************
//
//! @brief Turn the supply for I2C pull ups off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffI2C(void){
	uint32_t success = 0;
	success += gpioWrite(ON_OFF_I2C,false);
	success += gpioWrite(2,false);
	return success;
}

//*****************************************************************************
//
//! @brief Turn on the supply for voltage divider & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnVDiv(void){
	uint32_t success = 0;
	success += am_hal_gpio_pinconfig(ON_OFF_VOLTAGEDIVIDER,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch voltage divider on/off
	success += gpioWrite(ON_OFF_VOLTAGEDIVIDER,true);
	return success;
}
	
//*****************************************************************************
//
//! @brief Turn the supply for voltage divider off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffVDiv(void){
	uint32_t success = 0;
	success += gpioWrite(ON_OFF_VOLTAGEDIVIDER,false);
	return success;
}

//*****************************************************************************
//
//! @brief Turn on the supply for LoRa & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnLoRa(void){
	uint32_t success = 0;
	success += am_hal_gpio_pinconfig(ON_OFF_LORA,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch LoRa on/off
	success += gpioWrite(ON_OFF_LORA,true);
	return success;
}
	
//*****************************************************************************
//
//! @brief Turn the supply for LoRa off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffLoRa(void){
	uint32_t success = 0;
	success += gpioWrite(ON_OFF_LORA,false);
	return success;
}

//*****************************************************************************
//
//! @brief Turn on the supply for 3.3V & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOn33(void){
	uint32_t success = 0;
	success += am_hal_gpio_pinconfig(ON_OFF_3_3,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch 3.3V on/off
	success += gpioWrite(ON_OFF_3_3,true);
	return success;
}
	
//*****************************************************************************
//
//! @brief Turn the supply for 3.3V off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOff33(void){
	uint32_t success = 0;
	success += gpioWrite(ON_OFF_3_3,false);
	return success;
}

//*****************************************************************************
//
//! @brief Executes a test to check that all components and peripherals are running correctly
//!
//! @return 0 if everything is running correctly, else if not
//*****************************************************************************
uint32_t boardTest(void){
	int i = 0;
	//i += device_I2C_init();
	i += turnOnI2C();
	
	uint32_t deviceID = 0;	
	uint8_t dataBuffer[4];
	uint8_t readBuffer[4];
	float temperature;
	float humidity;
	
	dataBuffer[0] = 0xFF;
	dataBuffer[1] = 0xAA;
	dataBuffer[2] = 0x77;
	dataBuffer[3] = 0x11;
	i += am_devices_am1805_read_id(&deviceID);
	i += am_devices_mb85rc64ta_blocking_write(&dataBuffer[0],0,4);
	i += am_devices_mb85rc64ta_blocking_read(&readBuffer[0],0,4);
	i += turnOffI2C();
	return i;
}


//*****************************************************************************
//
//! @brief sets into normal sleep with full memory retention
//*****************************************************************************
void sleep_full_mem(void){			// ~3µA
	am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_NORMALSLEEP);
}
//*****************************************************************************
//
//! @brief sets into deep sleep with full memory retention
//*****************************************************************************
void deep_sleep_full_mem(void){		// ~3µA
	am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_DEEPSLEEP);
}
//*****************************************************************************
//
//! @brief Sets into sleep mode with 512k/32k memory retention
//!
//! @return 0 if everything worked fine, else for error
//*****************************************************************************
uint32_t sleep_half_mem(void){		//1.6µA
	int i = 0;
	i += am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_FLASH_MIN);
	// For optimal Deep Sleep current, configure cache to be powered-down in deepsleep:
    i += am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_CACHE);
    //
    // Power down SRAM, only 32K SRAM retained
    //
    i += am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_SRAM_MAX);
    i += am_hal_pwrctrl_memory_deepsleep_retain(AM_HAL_PWRCTRL_MEM_SRAM_32K_DTCM);
	
	am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_NORMALSLEEP);
	return i;
}

//*****************************************************************************
//
//! @brief Sets into deep sleep mode with 512k/32k memory retention
//!
//! @return 0 if everything worked fine, else for error
//*****************************************************************************
uint32_t deep_sleep_half_mem(void){		//~1.5µA
	int i = 0;
	i += am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_FLASH_MIN);
	// For optimal Deep Sleep current, configure cache to be powered-down in deepsleep:
    i += am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_CACHE);
    //
    // Power down SRAM, only 32K SRAM retained
    //
    i += am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_SRAM_MAX);
    i += am_hal_pwrctrl_memory_deepsleep_retain(AM_HAL_PWRCTRL_MEM_SRAM_32K_DTCM);
	
	am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_DEEPSLEEP);
	return i;
}

//! Arbitrary page address in flash instance 1.
#define ARB_PAGE_ADDRESS (AM_HAL_FLASH_INSTANCE_SIZE + (2 * AM_HAL_FLASH_PAGE_SIZE))
