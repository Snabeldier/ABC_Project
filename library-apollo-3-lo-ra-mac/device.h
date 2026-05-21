/*!
 *  \file       device.h
 *
 *  \brief      Provides power control for external devices by operating switches
 *
 *  \date       16.02.2023
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
 
#ifndef DEVICE_H
#define DEVICE_H
 
 
 //***** Header Files **********************************************************
#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "pins.h"
#include "gpio.h"
#include "dac63002.h"
#include "mb85rc64ta.h"
#include "arm_math.h"
#include "am_util_stdio.h"
#include "adc.h"
#include "timing.h"
#include "shtc3.h"
#include "am1805.h"
#include "as7341.h"
#include "taskHandler.h"
#include "ina232.h"
//#include "sht4x_i2c.h"


//*****************************************************************************
//
// Global definitions for the commands
//
//*****************************************************************************
#define LEN 512

//#define PAM_OPTIMIZED
//#define PAM_DMA

#define B0 -9.8853
#define B1 0.0046
#define B2 0.0136
#define B3 0.0243
#define B4 0.0459
#define B5 -0.0471
#define B6 0.0195
#define B7 0.0178
#define B8 -0.0026




//*****************************************************************************
//
// Function Prototypes
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Turn on the supply for FRAM & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnFram(void);
	
//*****************************************************************************
//
//! @brief Turn the supply for FRAM off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffFram(void);

//*****************************************************************************
//
//! @brief Turn on the supply for I2C pull ups & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnI2C(void);
	
//*****************************************************************************
//
//! @brief Turn the supply for I2C pull ups off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffI2C(void);

//*****************************************************************************
//
//! @brief Turn on the supply for DAC & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnDAC(void);
	
//*****************************************************************************
//
//! @brief Turn the supply for DAC off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffDAC(void);

//*****************************************************************************
//
//! @brief Turn on the supply for AS7341 & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnPARLAI(void);
	
//*****************************************************************************
//
//! @brief Turn the supply for AS7341 off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffPARLAI(void);

//*****************************************************************************
//
/*//! @brief Turn on the supply for SHT4x & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnTemp(void);*/
// Changed for ForestTrials
//*****************************************************************************
//
//! @brief Turn on the supply for SHTC3 & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnTemp(void);

//*****************************************************************************
//
//! @brief Turn the supply for SHTC3 off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffTemp(void);

//*****************************************************************************
//
//! @brief Turn on the supply for voltage divider & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnVDiv(void);
	
//*****************************************************************************
//
//! @brief Turn the supply for voltage divider off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffVDiv(void);

//*****************************************************************************
//
//! @brief Turn on the supply for INA234 & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnPWRSensor(void);
	
//*****************************************************************************
//
//! @brief Turn the supply for INA234 off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffPWRSensor(void);

//*****************************************************************************
//
//! @brief Turn on the supply for LoRa & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnLoRa(void);
	
//*****************************************************************************
//
//! @brief Turn the supply for LoRa off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffLoRa(void);

//*****************************************************************************
//
//! @brief Turn on the supply for PAM & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOnPAM(void);
	
//*****************************************************************************
//
//! @brief Turn the supply for PAM off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOffPAM(void);

//*****************************************************************************
//
//! @brief Turn on the supply for 3.3V & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOn33(void);
	
//*****************************************************************************
//
//! @brief Turn the supply for 3.3V off & return whether it was successful or not
//!
//! @return 32-bit success
//
//*****************************************************************************
uint32_t turnOff33(void);
//*****************************************************************************
//
//! @brief Initializes the DAC to output a voltage
//!
//! @return 32-bit success
//*****************************************************************************
uint32_t initDAC();

//*****************************************************************************
//
//! @brief Executes board tests to check if all ICs are correctly working
//!		   
//!
//! @return 32-bit error marker
//*****************************************************************************
uint32_t boardTest(void);


//*****************************************************************************
//
//! @brief Executes temperature/humidity measurement: used for final testing
//!		   
//!
//! @return 32-bit error marker
//*****************************************************************************
uint32_t TempHumid_Meas_run(void);


//*****************************************************************************
//
//! @brief Sets controller in normal sleep mode with all memories enabled: used for final testing
//!		   
//*****************************************************************************
void sleep_full_mem(void);

//*****************************************************************************
//
//! @brief Sets controller in deep sleep mode with all memories enabled: used for final testing
//!		   
//*****************************************************************************
void deep_sleep_full_mem(void);

//*****************************************************************************
//
//! @brief Sets controller in normal sleep mode with 512K/32K: used for final testing
//!		   
//!
//! @return 32-bit error marker
//*****************************************************************************
uint32_t sleep_half_mem(void);

//*****************************************************************************
//
//! @brief Sets controller in deep sleep mode with 512K/32K: used for final testing
//!		   
//!
//! @return 32-bit error marker
//*****************************************************************************
uint32_t deep_sleep_half_mem(void);



uint32_t getAdcTempMeasure(void);

//*****************************************************************************
//
//! @brief Enables power for all sensors and initializes the AS7341 & INA232
//!		   
//!
//! @return 32-bit error marker
//*****************************************************************************
uint32_t prepareSensing(void);

#endif