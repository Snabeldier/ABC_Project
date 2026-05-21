/***************************************************************************//**
 *  \file       taskHandler.h
 *
 *  \brief      Provides multiple functions for tasks used in the main.
 *
 *  \date       06.03.2024
 *
 *  \author     Uttunga Shinde ( Uni Freiburg IMTEK )
 ******************************************************************************/

#ifndef TASKS_TASKHANDLER_H_
#define TASKS_TASKHANDLER_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "apollo3.h"
#include "am_mcu_apollo.h"
#include "device.h"
#include "mb85rc64ta.h"
#include "i2c.h"
#include "spi.h"
#include "dac63002.h"
#include "am1805.h"
#include "shtc3.h"
#include "as7341.h"
#include "timing.h"
#include "am_util_delay.h"
//#include "adc.h"
#include "am_util_stdio.h"
#include "PamMeasurements.h"
#include "sht4.h"
//#include "sht4x_i2c.h"


#define NUMBER_SEND 						67
#define CHANNEL_PAR 						0
#define CHANNEL_SHUNT_V 				1
#define CHANNEL_BUS_V 					2
#define CHANNEL_CURRENT_REG 		3
#define CHANNEL_SUPERCAP_V 			4
#define CHANNEL_PAM 						5
#define CHANNEL_SPECTRAL				6

#define TARGET_MAX_VALUE 				8000
#define TARGET_MIN_VALUE 				1000
#define MAX_AUTOGAIN_ITERATIONS 9




 typedef union SendData_u
{
		uint8_t SendBytes[NUMBER_SEND];
    struct
    {
				// PAR
				union
				{
					float PAR;
					uint8_t PARbytes[4];
				};

        // Spectral Sensor
				union 		
				{
						uint8_t spectral_bytes[24];
						struct
						{
								// Spectral Sensor
								uint16_t spec_ch0;
								uint16_t spec_ch1;
								uint16_t spec_ch2;
								uint16_t spec_ch3;
								uint16_t spec_ch4;
								uint16_t spec_ch5;
								uint16_t spec_ch6;
								uint16_t spec_ch7;
								uint16_t spec_clear;
								uint16_t spec_nir;
								uint16_t spec_gain;
								uint16_t spec_tint;
						}spectral_data;

				};
					
				// SHTx4, Temperature and Humidity
				        union
        {
            float temperature1;
            uint8_t temp1_bytes[4];
        };

        union
        {
            float humidity1;
            uint8_t hum1_bytes[4];
        };
				
				union
				{
            float temperature2;
            uint8_t temp2_bytes[4];
        };

        union
        {
            float humidity2;
            uint8_t hum2_bytes[4];
        };
				
				union
				{
            float temperature3;
            uint8_t temp3_bytes[4];
        };

        union
        {
            float humidity3;
            uint8_t hum3_bytes[4];
        };

				// Datetime
				union
				{
						//Calendar datetime;	//had to adopt this to different RTC
						time_struct datetime;
						uint8_t datetime_bytes[9];	//in memory this is 12 because of byte padding
				};
				
				union
				{
						float shuntVoltage;
						uint8_t shuntVoltage_bytes[4];
				};
				
				union
				{	
						float busVoltage;
						uint8_t busVoltage_bytes[4];
				};
				
				union 
				{
						float currentRegister;
						uint8_t currentRegister_bytes[4];
				};
				union 
				{
						float superCapVoltage;
						uint8_t superCapVoltage_bytes[4];
				};

				union 
				{
						int PAMYield;
						uint8_t PAMYield_bytes[2];
				};
				
				union 
				{
						int PAM_Fm;
						uint8_t PAM_Fm_bytes[2];
				};
				
				union 
				{
						int PAM_F;
						uint8_t PAM_F_bytes[2];
				};
				
				union 
				{
						float threshold;
						uint8_t threshold_bytes[4];
				};
				
				union 
				{
						int fl_ctr;	//When it is = 20, measure fluorescence, otherwise only do other measurements.
						uint8_t fl_ctr_bytes[4];
				};
				
    }Measurement;

}SendData_t;
 




typedef struct
{
		// Spectral Sensor

		float basic_ch0;
		float basic_ch1;
		float basic_ch2;
		float basic_ch3;
		float basic_ch4;
		float basic_ch5;
		float basic_ch6;
		float basic_ch7;
		float basic_clear;
		float basic_nir;
		float basic_gain;
		float basic_tint;

}MeasurementBasic_t;
	
/*
 *	@brief Initializes the I/O for measurements using the AS7341
 *
 *	@return 0 on success
 */
uint32_t initSpecMeasurement(void);


/*
 *	@brief Function that can be called to execute a measurement using the AS7341
 *
 *	@return 0 on success
 */
uint32_t executeSpecMeasurement(SendData_t *data);

void autogainSpec(as7341_gain_t last_gain, as7341_gain_t *new_gain, uint16_t last_clear);

//changed for ForestTrails
/*
 *	@brief Function that can be called to execute a measurement using the SHT4x
 * 
 *	@return 0 on success
*/

uint32_t executeTempMeasurement(SendData_t *data);

//changed for ForestTrails
/*
 *	@brief Function that can be called to execute a measurement using the SHTC3
 * 
 *	@return 0 on success
*/
uint32_t executeTempMeasurement(SendData_t *data);

	
//*****************************************************************************
//
//! @brief Executes the PAR calculation
//!
//! @return PAR value
//*****************************************************************************
float PARCalc(SendData_t *data);


void executePAMMeasurement(SendData_t *data);
//*****************************************************************************
//
//! @brief Executes the sensor measurements
//!
//*****************************************************************************
void measurementProcess(SendData_t *data);
 
 #endif /* TASKS_TASKHANDLER_H_ */