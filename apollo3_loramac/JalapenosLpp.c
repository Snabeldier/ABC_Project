/*!
 * \file      JalapenosLpp.c
 *
 * \brief     Extends the Cayenne Low Power Protocol to support float32
 #todo		add different data types
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _ 	             | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author 	  Timm Luhmann (IMTEK)
 */
#include <stdint.h>

#include "utilities.h"
#include "JalapenosLpp.h"

#define Jalapenos_LPP_MAXBUFFER_SIZE                  242

static uint8_t JalapenosLppBuffer[Jalapenos_LPP_MAXBUFFER_SIZE];
static uint8_t JalapenosLppCursor = 0;

void JalapenosLppInit( void )
{
    JalapenosLppCursor = 0;
}

void JalapenosLppReset( void )
{
    JalapenosLppCursor = 0;
}

uint8_t JalapenosLppGetSize( void )
{
    return JalapenosLppCursor;
}

uint8_t* JalapenosLppGetBuffer( void )
{
    return JalapenosLppBuffer;
}

uint8_t JalapenosLppCopy( uint8_t* dst )
{
    memcpy1( dst, JalapenosLppBuffer, JalapenosLppCursor );

    return JalapenosLppCursor;
}


uint8_t JalapenosLppAddDigitalInput( uint8_t channel, uint8_t value )
{
    if( ( JalapenosLppCursor + LPP_DIGITAL_INPUT_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_DIGITAL_INPUT; 
    JalapenosLppBuffer[JalapenosLppCursor++] = value; 

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddDigitalOutput( uint8_t channel, uint8_t value )
{
    if( ( JalapenosLppCursor + LPP_DIGITAL_OUTPUT_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_DIGITAL_OUTPUT; 
    JalapenosLppBuffer[JalapenosLppCursor++] = value; 

    return JalapenosLppCursor;
}


uint8_t JalapenosLppAddAnalogInput( uint8_t channel, float value )
{
    if( ( JalapenosLppCursor + LPP_ANALOG_INPUT_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    union analogVal val;
	val.analog = value;
    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_ANALOG_INPUT; 
	JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[3];
	JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[2];
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddAnalogOutput( uint8_t channel, float value )
{
    if( ( JalapenosLppCursor + LPP_ANALOG_OUTPUT_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int16_t val = ( int16_t ) ( value * 100 );
    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_ANALOG_OUTPUT;
    JalapenosLppBuffer[JalapenosLppCursor++] = val >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val; 

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddDeviceID ( uint32_t deviceID){

	if( ( JalapenosLppCursor + LPP_DEVICEID_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
		
		union analogVal ID;
		ID.analog = deviceID;
		
		JalapenosLppBuffer[JalapenosLppCursor++] = ID.bytes[0];
		JalapenosLppBuffer[JalapenosLppCursor++] = ID.bytes[1];
		JalapenosLppBuffer[JalapenosLppCursor++] = ID.bytes[2];

    return JalapenosLppCursor;	
}

uint8_t JalapenosLppAddDummyData() {

	if(( JalapenosLppCursor + LPP_DUMMY_DATA_SIZE) > Jalapenos_LPP_MAXBUFFER_SIZE ) {
        return 0;
    }
		
		union analogVal dummy;
		dummy.analog = 12345;
		
		JalapenosLppBuffer[JalapenosLppCursor++] = dummy.bytes[0];
		JalapenosLppBuffer[JalapenosLppCursor++] = dummy.bytes[1];
		JalapenosLppBuffer[JalapenosLppCursor++] = dummy.bytes[2];

    return JalapenosLppCursor;	
}

uint8_t JalapenosLppAddTemperatureAndHumidity(float temp, float humid) {

	if(( JalapenosLppCursor + LPP_TEMP_AND_HUMID_SIZE) > Jalapenos_LPP_MAXBUFFER_SIZE ) {
        return 0;
    }

    int16_t tempVal  = (int16_t)(temp  * 100);
    int16_t humidVal = (int16_t)(humid * 100);

    JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(tempVal);
    JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(tempVal  >> 8);

    JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(humidVal);
    JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(humidVal >> 8);

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddTimestamp ( uint8_t channel, uint32_t time){
	
	if( ( JalapenosLppCursor + LPP_TIMESTAMP_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }

    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_TIMESTAMP; 
	JalapenosLppBuffer[JalapenosLppCursor++] = time >> 24;
	JalapenosLppBuffer[JalapenosLppCursor++] = time >> 16;
    JalapenosLppBuffer[JalapenosLppCursor++] = time >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = time; 

    return JalapenosLppCursor;	
}

uint8_t JalapenosLppAddShuntVoltage ( uint8_t channel, float shuntVoltage){
	
	if( ( JalapenosLppCursor + LPP_SHUNTVOLTAGE_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    union analogVal val;
	val.analog = shuntVoltage;
    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_SHUNTVOLTAGE; 
	JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[3];
	JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[2];
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 

    return JalapenosLppCursor;	
}

uint8_t JalapenosLppAddBusVoltage ( uint8_t channel, float busVoltage){
	
	if( ( JalapenosLppCursor + LPP_BUSVOLTAGE_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    union analogVal val;
	val.analog = busVoltage;
    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_BUSVOLTAGE; 
	JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[3];
	JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[2];
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 

    return JalapenosLppCursor;	
}

uint8_t JalapenosLppAddCurrent ( uint8_t channel, float current){
	
	if( ( JalapenosLppCursor + LPP_CURRENT_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    union analogVal val;
	val.analog = current;
    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_CURRENT; 
	JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[3];
	JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[2];
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 

    return JalapenosLppCursor;	
}

uint8_t JalapenosLppAddPowerMeasurement(float shuntVoltage_mV, float busVoltage_V, float current_uA)
{
    if ((JalapenosLppCursor + LPP_POWER_MEASUREMENT_SIZE) > Jalapenos_LPP_MAXBUFFER_SIZE)
        return 0;

    /* shuntVoltage: 0.01 mV resolution -> range +-327.67 mV (covers +-320 mV INA219 range) */
    int16_t shuntVal   = (int16_t)(shuntVoltage_mV * 100.0f);
    /* busVoltage: 1 mV resolution -> range 0-65.535 V */
    int16_t busVal     = (int16_t)(busVoltage_V * 1000.0f);
    /* current: 1 uA resolution -> range +-32.767 mA */
    int16_t currentVal = (int16_t)(current_uA);

    JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(shuntVal);
    JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(shuntVal >> 8);

    JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(busVal);
    JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(busVal >> 8);

    JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(currentVal);
    JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(currentVal >> 8);

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddCurrentBusArray(const int16_t *current, const int16_t *bus, uint8_t count)
{
    if ((JalapenosLppCursor + LPP_POWER_ARRAY_HEADER_SIZE + (uint16_t)count * LPP_POWER_ARRAY_SAMPLE_SIZE) > Jalapenos_LPP_MAXBUFFER_SIZE)
        return 0;

    /* Number of samples that follow */
    JalapenosLppBuffer[JalapenosLppCursor++] = count;

    /* Per sample: current (0.1 mA/LSB), bus voltage (1 mV/LSB).
     * Each value as signed 16-bit, little-endian (LSB first). */
    for (uint8_t i = 0; i < count; i++)
    {
        JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(current[i]);
        JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(current[i] >> 8);

        JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(bus[i]);
        JalapenosLppBuffer[JalapenosLppCursor++] = (uint8_t)(bus[i] >> 8);
    }

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddLedAndValue(uint8_t ledState, uint8_t value)
{
    if ((JalapenosLppCursor + LPP_LED_AND_VALUE_SIZE) > Jalapenos_LPP_MAXBUFFER_SIZE)
        return 0;

    JalapenosLppBuffer[JalapenosLppCursor++] = ledState ? 1 : 0;
    JalapenosLppBuffer[JalapenosLppCursor++] = value;

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddCapVoltage ( uint8_t channel, float CapVoltage){

	
	
	
	if( ( JalapenosLppCursor + LPP_CAPVOLTAGE_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    union analogVal val;
	val.analog = CapVoltage;
    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_CAPVOLTAGE; 
	JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[3];
	JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[2];
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 

    return JalapenosLppCursor;	
}