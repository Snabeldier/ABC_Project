/*!
 * \file      JalapenosLpp.h
 *
 * \brief     Extends the Cayenne Low Power Protocol to support float 32
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */
#ifndef __JALAPENOS_LPP_H__
#define __JALAPENOS_LPP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Type codes: each value identifies the sensor type in the binary payload (1 byte before the value).
 * Codes 0-3 are Cayenne LPP standard (kept for compatibility, unused in this project).
 * Codes 4-105 are reserved by Cayenne (e.g. 103=barometer, 104=gyroscope) — not used here.
 * Codes 106-114 are custom JalapenosLpp extensions for sensors not covered by Cayenne.
 * SIZE constants include all bytes for one entry: 1 (channel) + 1 (type code) + n (value). */
#define LPP_DIGITAL_INPUT       			0       // 1 byte
#define LPP_DIGITAL_OUTPUT      			1       // 1 byte
#define LPP_ANALOG_INPUT        			2       // 2 bytes, 0.01 signed
#define LPP_ANALOG_OUTPUT       			3       // 2 bytes, 0.01 signed
#define LPP_TIMESTAMP									106			// 4 bytes, unsigned date
#define LPP_SHUNTVOLTAGE							107			// 4 bytes, signed value
#define LPP_BUSVOLTAGE								108			// 4 bytes, signed value
#define LPP_CURRENT										109			// 4 bytes, signed value
#define LPP_CAPVOLTAGE								110			// 4 bytes, signed value
#define LPP_DEVICEID									114			// 3 bytes, signed value

// Data ID + Data Type + Data Size
#define LPP_DIGITAL_INPUT_SIZE       	3
#define LPP_DIGITAL_OUTPUT_SIZE      	3
#define LPP_ANALOG_INPUT_SIZE        	6
#define LPP_ANALOG_OUTPUT_SIZE       	6
#define LPP_TEMP_AND_HUMID_SIZE				4
#define LPP_DEVICEID_SIZE							3
#define LPP_DUMMY_DATA_SIZE						3

#define LPP_TIMESTAMP_SIZE						6
#define LPP_SHUNTVOLTAGE_SIZE					6
#define LPP_BUSVOLTAGE_SIZE						6
#define LPP_CURRENT_SIZE							6
#define LPP_CAPVOLTAGE_SIZE						6
#define LPP_POWER_MEASUREMENT_SIZE		6   /* shuntVoltage(int16) + busVoltage(uint16) + current(int16) */
#define LPP_POWER_ARRAY_HEADER_SIZE		1   /* sample count byte preceding the samples */
#define LPP_POWER_ARRAY_SAMPLE_SIZE		4   /* per sample: current(int16) + bus(int16)  */


union analogVal
{
	int analog;
	uint8_t bytes[sizeof(float)];
};

void JalapenosLppInit( void );

void JalapenosLppReset( void );
uint8_t JalapenosLppGetSize( void );
uint8_t* JalapenosLppGetBuffer( void );
uint8_t JalapenosLppCopy( uint8_t* buffer );

uint8_t JalapenosLppAddDigitalInput( uint8_t channel, uint8_t value );
uint8_t JalapenosLppAddDigitalOutput( uint8_t channel, uint8_t value );

uint8_t JalapenosLppAddAnalogInput( uint8_t channel, float value );
uint8_t JalapenosLppAddAnalogOutput( uint8_t channel, float value );

uint8_t JalapenosLppAddDeviceID ( uint32_t deviceID);

uint8_t JalapenosLppAddTemperatureAndHumidity(float temp, float humid);
uint8_t JalapenosLppAddDummyData ( void );

uint8_t JalapenosLppAddTimestamp ( uint8_t channel, uint32_t time);
uint8_t JalapenosLppAddShuntVoltage ( uint8_t channel, float shuntVoltage);
uint8_t JalapenosLppAddBusVoltage ( uint8_t channel, float busVoltage);
uint8_t JalapenosLppAddCurrent ( uint8_t channel, float current);
uint8_t JalapenosLppAddCapVoltage ( uint8_t channel, float CapVoltage);

/* Compact INA219 payload: shuntVoltage (0.01 mV), busVoltage (1 mV), current (1 uA).
 * All stored little-endian as signed 16-bit integers. */
uint8_t JalapenosLppAddPowerMeasurement(float shuntVoltage_mV, float busVoltage_V, float current_uA);

/* Power array: 1 count byte followed by <count> samples. Each sample is two
 * signed 16-bit little-endian values in this order:
 *   current (0.1 mA/LSB), bus voltage (1 mV/LSB).
 * Used for both the sleep/baseline trace and the TX-burst trace. A count of 0
 * means no data is available yet (e.g. first uplink of a session). */
uint8_t JalapenosLppAddCurrentBusArray(const int16_t *current, const int16_t *bus, uint8_t count);

#ifdef __cplusplus
}
#endif

#endif // __JALAPENOS_LPP_H__
