/*!
 *  \file       boardControl.h
 *
 *  \brief      Provides library access and functions to control & test the system
 *
 *  \date       02.02.2023
 *
 *  \author     Timm Luhmann (IMTEK)
 */
 
#ifndef BOARDCONTROL_H
#define BOARDCONTROL_H

#include <stdint.h>
#include "am1805.h"
#include "as7341.h"
#include "dac63002.h"
#include "i2c.h"
#include "spi.h"
#include "shtc3.h"
#include "mb85rc64ta.h"
#include "am_util_delay.h"

#define LEN 512

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
//! @brief Executes board tests to check if all ICs are correctly working
//!		   
//!
//! @return 32-bit error marker
//*****************************************************************************
uint32_t boardTest(void);

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

//*****************************************************************************
//
//! @brief Initializes the DAC to output a voltage
//!
//! @return 32-bit success
//*****************************************************************************
uint32_t initDAC();

#endif