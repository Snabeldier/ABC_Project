/*!
 *  \file       boardControl.c
 *
 *  \brief      Provides functions to control the board components
 *
 *  \date       09.11.2023
 *
 *  \author     Timm Luhmann (IMTEK)
 */
//***** Header Files **********************************************************
//#include <stdint.h>
//#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "pins.h"
#include "gpio.h"
#include "pins.h"
#include "boardControl.h"


//*****************************************************************************
//
// Function definitions
//
//*****************************************************************************


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
//! @brief Turn on the supply for DAC & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnDAC(void){
	uint32_t success = 0;
	success += am_hal_gpio_pinconfig(ON_OFF_DAC,g_AM_HAL_GPIO_OUTPUT_8);          //Pin to switch DAC on/off
	success += gpioWrite(ON_OFF_DAC,true);
	return success;
}
	
//*****************************************************************************
//
//! @brief Turn the supply for DAC off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffDAC(void){
	uint32_t success = 0;
	success += gpioWrite(ON_OFF_DAC,false);
	return success;
}



//*****************************************************************************
//
//! @brief Turn on the supply for AS7341 & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnPARLAI(void){
	uint32_t success = 0;
	success += am_hal_gpio_pinconfig(ON_OFF_PARLAI,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch AS7341 on/off
	success += gpioWrite(ON_OFF_PARLAI,true);
	return success;
}
	
//*****************************************************************************
//
//! @brief Turn the supply for AS7341 off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffPARLAI(void){
	uint32_t success = 0;
	success += gpioWrite(ON_OFF_PARLAI,false);
	return success;
}

//*****************************************************************************
//
//! @brief Turn on the supply for SHTC3 & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnTemp(void){
	uint32_t success = 0;
	success += am_hal_gpio_pinconfig(ON_OFF_TEMP,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch SHTC3 on/off
	success += gpioWrite(ON_OFF_TEMP,true);
	return success;
}
	
//*****************************************************************************
//
//! @brief Turn the supply for SHTC3 off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffTemp(void){
	uint32_t success = 0;
	success += gpioWrite(ON_OFF_TEMP,false);
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
//! @brief Turn on the supply for INA234 & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnPWRSensor(void){
	uint32_t success = 0;
	success += am_hal_gpio_pinconfig(ON_OFF_POWERSENSOR,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch INA234 on/off
	success += gpioWrite(ON_OFF_POWERSENSOR,true);
	return success;
}
	
//*****************************************************************************
//
//! @brief Turn the supply for INA234 off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffPWRSensor(void){
	uint32_t success = 0;
	success += gpioWrite(ON_OFF_POWERSENSOR,false);
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
//! @brief Turn on the supply for PAM & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnPAM(void){
	uint32_t success = 0;
	success += am_hal_gpio_pinconfig(ON_OFF_PAM,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch PAM on/off
	success += gpioWrite(ON_OFF_PAM,true);
	return success;
}
	
//*****************************************************************************
//
//! @brief Turn the supply for PAM off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffPAM(void){
	uint32_t success = 0;
	success += gpioWrite(ON_OFF_PAM,false);
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
	i += device_I2C_init();
	i += turnOnDAC();
	i += turnOnFram();
	i += turnOnI2C();
	i += turnOnPARLAI();
	i += turnOnTemp();
	i += turnOnPAM();
	
	uint32_t deviceID = 0;	
	uint8_t dataBuffer[4];
	uint8_t readBuffer[4];
	float temperature;
	float humidity;
	dataBuffer[0] = 0xFF;
	dataBuffer[1] = 0xAA;
	dataBuffer[2] = 0x77;
	dataBuffer[3] = 0x11;
	i += as7341_testCommunication();
	i += am_devices_am1805_read_id(&deviceID);
	i += am_devices_dac63002_read_id(&deviceID);
	i += am_devices_mb85rc64ta_blocking_write(&dataBuffer[0],0,4);
	i += am_devices_mb85rc64ta_blocking_read(&readBuffer[0],0,4);
	i += initDAC();
	i += am_devices_dac63002_1_set_output(0xAAA0);
	i += SHTC3_GetTempAndHumi(&temperature,&humidity);
	i += turnOffDAC();
	i += turnOffFram();
	i += turnOffI2C();
	i += turnOffPARLAI();
	i += turnOffTemp();
	i += turnOffPAM();
	return i;
}

//*****************************************************************************
//
//! @brief sets into normal sleep with full memory retention
//*****************************************************************************
void sleep_full_mem(void){			// ~3A
	am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_NORMALSLEEP);
}
//*****************************************************************************
//
//! @brief sets into deep sleep with full memory retention
//*****************************************************************************
void deep_sleep_full_mem(void){		// ~3A
	am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_DEEPSLEEP);
}
//*****************************************************************************
//
//! @brief Sets into sleep mode with 512k/32k memory retention
//!
//! @return 0 if everything worked fine, else for error
//*****************************************************************************
uint32_t sleep_half_mem(void){		//1.6A
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
uint32_t deep_sleep_half_mem(void){		//~1.5A
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

//*****************************************************************************
//
//! @brief Initializes the DAC to output a voltage
//!
//! @return 32-bit success
//*****************************************************************************
uint32_t initDAC(){
	uint32_t success = 0;
	uint32_t data;
	uint32_t deviceID;
//	data = DEVUNLOCK + RESET;
//	//data = dataPrepare(data);
//	success += am_devices_dac63002_1_set_common_trigger(pHandle,dataPrepare(data));
//	success += am_devices_dac63002_read_register(pHandle,&deviceID,0x1F);
//	data = POWER_UP_VOUT + POWER_DOWN_VOUT0_HIZ + POWER_DOWN_IOUT + ENABLE_INTERNAL_REF;
//	success += am_devices_dac63002_1_set_common_config(pHandle,data);
//	success += am_devices_dac63002_1_set_gain(pHandle,GAIN_1_5_INTERNAL);		//Use VDD as reference
//	success += am_devices_dac63002_1_disable_CMP(pHandle);				//disable internal comparator
//	success += am_devices_dac63002_1_set_output( 0x5540);		//set output to 0.6V
	success += am_devices_dac63002_1_set_common_config(0x1E01);	//Power up voltage on channel 1 & enable internal reference
	success += am_devices_dac63002_1_set_gain(GAIN_1_5_INTERNAL);	//set channel gain to 1.5 Internal Ref (-> 1.8V)
	//success += am_devices_dac63002_write_register(0x0800,ADDRESS_DAC1_VOUT_CMP_CONFIG);	//Set channel 1 gain to 1.5 Internal Reference (1.8)
	//success += am_devices_dac63002_write_register(pHandle,0x0135,0x24);
	success += am_devices_dac63002_1_linear_slew(CODE_STEP_2, SLEW_RATE_60);	//Slewrate of 60?S, 2 LSB code step
	//success += am_devices_dac63002_write_register(0x0017,ADDRESS_DAC1_FUNC_CONFIG);	//Slewrate of 60?S, 2 LSB code step
	success += am_devices_dac63002_write_register(0xA540,ADDRESS_DAC1_MARGIN_HIGH);	//DAC margin high code(1.164V) for 1.8V
	success += am_devices_dac63002_write_register(0x0500,ADDRESS_DAC1_MARGIN_LOW);	//DAC low margin (36mV) for 1.8V	
	success += am_devices_dac63002_write_register(0x0002,ADDRESS_COMMON_TRIGGER);	//Save settings to NVM
	success += am_devices_dac63002_1_set_Iout_Range(IOUT_0_25);

	return success;	
}
