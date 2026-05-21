/***************************************************************************//**
 *  \file       taskHandler.c
 *
 *  \brief      Provides multiple functions for tasks used in the main.
 *
 *  \date       06.03.2024
 *
 *  \author     Uttunga Shinde ( Uni Freiburg IMTEK )
 *
 *  \author     Johannes Klueppel ( Uni Freiburg IMTEK )
 ******************************************************************************/
 
 #include "taskHandler.h"
 
 /******************************************************************************
 * VARIABLES
 *****************************************************************************/

static bool parInitialized = false;
static as7341_gain_t new_gain = AS7341_AGAIN_512;
/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/
uint32_t SpectrometerMeasurement(uint16_t *data);



/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/
/***************************************************************************//**
 * \brief Functions that triggers the measurement of the spectral Sensor
 *
 * \param [in] selectgain If selectgain=-1 --> gain is according to our own specification: Ch1 - 8: Gain 4, rest: Gain1
 *
 * \todo    return value
 ******************************************************************************/
 //#define DEBUG_AS
 /***************************************************************************//**
 *
 * \brief Initializes I2C
 *
 ******************************************************************************/
uint32_t initSpecMeasurement(void){
	uint32_t error = 0;
	error += device_I2C_init();		//this is known to cause an error if called a second time without de-init
	parInitialized = true;
	return error;
}

/***************************************************************************//**
 * \brief Executes the whole measurement of the spectral sensor. 
 * 				Starts the autogain function.
 *				Moves the data to the measurements struct.
 *
 * \param [in] MeasurementData_t struct
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
uint32_t executeSpecMeasurement(SendData_t *data){
	uint32_t error = 0;
	uint16_t spectralData[12];
	
	// Initialize I2C if not already done
	if(parInitialized==false){
		error += initSpecMeasurement();
	}
	
	// Test connection to sensor
	error += as7341_testCommunication();
	if (error)
	{
		// If no sensors answers, transfer error values in the measurement struct
		data->Measurement.spectral_data.spec_ch0 		= 0xFBFF;
		data->Measurement.spectral_data.spec_ch1 		= 0xFBFF;
		data->Measurement.spectral_data.spec_ch2 		= 0xFBFF;
		data->Measurement.spectral_data.spec_ch3 		= 0xFBFF;
		data->Measurement.spectral_data.spec_ch4 		= 0xFBFF;
		data->Measurement.spectral_data.spec_ch5 		= 0xFBFF;
		data->Measurement.spectral_data.spec_ch6 		= 0xFBFF;
		data->Measurement.spectral_data.spec_ch7 		= 0xFBFF;
		data->Measurement.spectral_data.spec_clear 	= 0xFBFF;
		data->Measurement.spectral_data.spec_nir 		= 0xFBFF;
		data->Measurement.spectral_data.spec_gain 	= 0xFBFF;
		data->Measurement.spectral_data.spec_tint 	= 0xFBFF;
		
		return error;
	}
	else
	{
		// Configure measurement parameter
		error += as7341_config(29, 599, new_gain);
		uint16_t gain = 0;
		
		// Check if clear value is inside the borders
		// If yes, continue with transfering the measurements
		// If no, set new gain, repeat measurement
		for (uint8_t i = 0; i < MAX_AUTOGAIN_ITERATIONS; i++)
    {
			error += SpectrometerMeasurement(spectralData);
			error += as7341_getGain(&gain);
		  uint16_t target_value = spectralData[8];

      if (target_value < TARGET_MAX_VALUE && target_value > TARGET_MIN_VALUE)
      {
        break;
      }
      else if(target_value < TARGET_MIN_VALUE && gain == AS7341_AGAIN_512)
      {
        break;
      }
      else if(target_value > TARGET_MAX_VALUE && gain == AS7341_AGAIN_05)
      {
        break;
      }
      else
      {
		autogainSpec(gain, &new_gain, target_value);
        as7341_setGain(new_gain);
      }
    }
			// Transfer data to Measurement struct
			data->Measurement.spectral_data.spec_ch0 = spectralData[0];
			data->Measurement.spectral_data.spec_ch1 = spectralData[1];
			data->Measurement.spectral_data.spec_ch2 = spectralData[2];
			data->Measurement.spectral_data.spec_ch3 = spectralData[3];
			data->Measurement.spectral_data.spec_ch4 = spectralData[4];
			data->Measurement.spectral_data.spec_ch5 = spectralData[5];
			data->Measurement.spectral_data.spec_ch6 = spectralData[6];
			data->Measurement.spectral_data.spec_ch7 = spectralData[7];
			data->Measurement.spectral_data.spec_clear = spectralData[8];
			data->Measurement.spectral_data.spec_nir = spectralData[10];
				
			error += as7341_getGain(&gain);
			error += as7341_convertGain(&gain);
			data->Measurement.spectral_data.spec_gain = (uint16_t) gain;
			double tint = 0;
			error += as7341_getTINT(&tint);
			data->Measurement.spectral_data.spec_tint = (uint16_t) tint;
			
			return error;
	}
 }

 /***************************************************************************//**
 * \brief Functions that triggers the measurement of the spectral sensor.
 *
 * \param [in] uint16_t *data of at least size 12
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
uint32_t SpectrometerMeasurement(uint16_t *data)
{
	uint32_t error = 0;	// typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
	
	// Configure Channel F1 to F6 to the ADC 
  error += as7341_writeSMUXmapping(AS7341_SMUX_F1_F6);
	// Wait for the task to be done
	error += as7341_delayForSMUX();
	// Start measuring configured channels
	error += as7341_startMeasurement(); 
	// Wait for the task to be done
	error += as7341_delayForData();
	// Read measurements from sensor
	error += as7341_transmitMeasurements(data);
	// Stop measurement
	error += as7341_stopMeasuring();

	// Configure Channel F7 and F8 as well as Clear, Dark, NIR, Flicker to the ADC 
	error += as7341_writeSMUXmapping(AS7341_SMUX_F7F8CDNF); 
	// Wait for the task to be done
	error += as7341_delayForSMUX();
	// Start measuring configured channels
	error += as7341_startMeasurement(); 
	// Wait for the task to be done
	error += as7341_delayForData();
	// Read measurements from sensor
	error += as7341_transmitMeasurements(data);
	// Stop measurement
	error += as7341_stopMeasuring();
		
	return error;
}

void autogainSpec(as7341_gain_t last_gain, as7341_gain_t *new_gain, uint16_t last_clear) {

    *new_gain = last_gain;

    // Clear > target value and gain > 1: reduce gain
    if ((last_clear > TARGET_MAX_VALUE) && (last_gain > AS7341_AGAIN_05)) {
        *new_gain = (as7341_gain_t)(last_gain - 1);
    }

    // Clear < target value and gain < 512: increase gain
    if ((last_clear < TARGET_MIN_VALUE) && (last_gain < AS7341_AGAIN_512)) {
        *new_gain = (as7341_gain_t)(last_gain + 1);
    }
}


	
//*****************************************************************************
//
//! @brief Executes the PAR calculation
//!
//! @return PAR value
//*****************************************************************************
float PARCalc(SendData_t *data){
	MeasurementBasic_t basic_values;
	
	as7341_convertGain(&data->Measurement.spectral_data.spec_gain);
	
	// Divide raw value with ADC gain and integration time
	float div = (data->Measurement.spectral_data.spec_gain * data->Measurement.spectral_data.spec_tint);
	basic_values.basic_ch0 = data->Measurement.spectral_data.spec_ch0 / div;
	basic_values.basic_ch1 = data->Measurement.spectral_data.spec_ch1 / div;
	basic_values.basic_ch2 = data->Measurement.spectral_data.spec_ch2 / div;
	basic_values.basic_ch3 = data->Measurement.spectral_data.spec_ch3 / div;
	basic_values.basic_ch4 = data->Measurement.spectral_data.spec_ch4 / div;
	basic_values.basic_ch5 = data->Measurement.spectral_data.spec_ch5 / div;
	basic_values.basic_ch6 = data->Measurement.spectral_data.spec_ch6 / div;
	basic_values.basic_ch7 = data->Measurement.spectral_data.spec_ch7 / div;
	basic_values.basic_clear = data->Measurement.spectral_data.spec_clear / div;
	basic_values.basic_nir = data->Measurement.spectral_data.spec_nir / div;
	
	float PAR = 0;
	float calibration_coefficients[8] = {59.78495321, 21.55701636, 19.45583406, 14.18760895, 12.18362329,  9.80367365, 11.1446049,  9.37706214}; //FlexPCB 1, Calib. 17.01.2025, not correct, too high
	PAR += basic_values.basic_ch0 * calibration_coefficients[0];
	PAR += basic_values.basic_ch1 * calibration_coefficients[1];
	PAR += basic_values.basic_ch2 * calibration_coefficients[2];
	PAR += basic_values.basic_ch3 * calibration_coefficients[3];
	PAR += basic_values.basic_ch4 * calibration_coefficients[4];
	PAR += basic_values.basic_ch5 * calibration_coefficients[5];
	PAR += basic_values.basic_ch6 * calibration_coefficients[6];
	PAR += basic_values.basic_ch7 * calibration_coefficients[7] * 0.73; // care for responsiveness above 700nm
	return PAR;
}

uint32_t executeTempMeasurement(SendData_t *data){
	uint32_t error = 0;
	float temps[3], hums[3];
	timerDelay(1);	
	error += SHT4x_GetTempAndHumi(temps, hums);
	if (error)
	{
		#if defined(HW_CF_SENSOR)
			data->Measurement.humidity1 = 101.0;
			data->Measurement.temperature1 = 100.0;
		#elif defined(HW_ECOVETTE)
			data->Measurement.humidity1 = 101.0;
			data->Measurement.temperature1 = 100.0;
			data->Measurement.humidity2 = 101.0;
			data->Measurement.temperature2 = 100.0;
			data->Measurement.humidity3 = 101.0;
			data->Measurement.temperature3 = 100.0;
		#endif
	}
	else
	{
		#if defined(HW_CF_SENSOR)
			data->Measurement.humidity1 = hums[0];
			data->Measurement.temperature1 = temps[0];
		#elif defined(HW_ECOVETTE)
			data->Measurement.humidity1 = hums[0];
			data->Measurement.temperature1 = temps[0];
			data->Measurement.humidity2 = hums[1];
			data->Measurement.temperature2 = temps[1];
			data->Measurement.humidity3 = hums[2];
			data->Measurement.temperature3 = temps[2];
		#endif
	}
	error += turnOffTemp();
	return error;
}


//PAM measurement EXE
void executePAMMeasurement(SendData_t *data){
	float minfluo;
	float maxfluo;
	float yield;
	uint16_t minfluo_int;
	uint16_t maxfluo_int;
	uint16_t yield_int;
	
	initPamMeasurement();
	DoPamMeasurement(&minfluo, &maxfluo, &yield);
	
	minfluo_int = minfluo * 1000;
	maxfluo_int = maxfluo * 1000;
	
	if (minfluo>maxfluo ||  yield > 2){
		yield = 2;
	}
	
	yield_int = yield * 1000;

	data->Measurement.PAM_F = minfluo_int;
	data->Measurement.PAM_Fm = maxfluo_int;
	data->Measurement.PAMYield = yield_int;
	
}

//*****************************************************************************
//
//! @brief Executes the PAR calculation
//!
//! @return PAR value
//*****************************************************************************

void measurementProcess(SendData_t *data){
	
	float threshold;
	
	prepareSensing();
	
	
	/*
	ina232_readBusVoltage(&data->Measurement.busVoltage);
	ina232_readShuntVoltage(&data->Measurement.shuntVoltage);
	ina232_readCurrent(&data->Measurement.currentRegister);
	executeSpecMeasurement(&measData);
	data->Measurement.PAR = PARCalc(&measData);
	*/
	
	executeTempMeasurement(data);
	
	executeSpecMeasurement(data);
	
	
#if defined(HW_CF_SENSOR)
	if (data->Measurement.spectral_data.spec_gain == 0xFBFF)
	{
		data->Measurement.threshold = 0xFBFF;
	}
	else
	{
		threshold = (float)data->Measurement.spectral_data.spec_clear / (float)(data->Measurement.spectral_data.spec_tint * (float)data->Measurement.spectral_data.spec_gain);
		data->Measurement.threshold = threshold;
	}
	
	
	
	//am_util_stdio_printf("Clear: %d\n",  data->Measurement.spectral_data.spec_clear);
	//am_util_stdio_printf("tint: %d\n",  data->Measurement.spectral_data.spec_tint);
	//am_util_stdio_printf("gain: %d\n",  data->Measurement.spectral_data.spec_gain);

	am_util_stdio_printf("Threshold: %f\n",  data->Measurement.threshold);

	
	//data->Measurement.superCapVoltage = 1.5 * 3.21 * getAdcSample() / 16384;		//1.5V is the ADC reference, 3.21 originates from the resistors, 2^14 resolution
		
	//data->Measurement.PAMYield = PAMYield(&dummy);
	
	//data->Measurement.PAMYield = PAMYield(&dummy);
	
	

	//The measurements such as PAM and Temp, are more frequently done than Chlorophyll Fluorescence
	
	//For day measurements, do fluorescence measurement once in 20 cycles
	if(data->Measurement.fl_ctr == 4 && data->Measurement.threshold >= 0.00078){	
		executePAMMeasurement(data);
		data->Measurement.fl_ctr = 1;
	}
	//For night measurements, do fluorescence measurement once in 60 cycles
	else if(data->Measurement.fl_ctr == 12){
		executePAMMeasurement(data);
		data->Measurement.fl_ctr = 1;
	}
	//Send dummy values when doing the other measurements and waiting for 20 or 60 cycles.
	
	//TO:DO Change the dummy parts.
	else{
		data->Measurement.fl_ctr++;
		data->Measurement.PAM_F = 5000;
		data->Measurement.PAM_Fm = 5000;
		data->Measurement.PAMYield = 1000;
	}
	am_util_stdio_printf("Fluorescence Measurement Counter: %d\n",  data->Measurement.fl_ctr);
#endif
	
	//data->
	
}	
	