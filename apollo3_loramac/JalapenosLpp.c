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


uint8_t JalapenosLppAddLuminosity( uint8_t channel, uint16_t lux )
{
    if( ( JalapenosLppCursor + LPP_LUMINOSITY_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_LUMINOSITY; 
    JalapenosLppBuffer[JalapenosLppCursor++] = lux >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = lux; 

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddPresence( uint8_t channel, uint8_t value )
{
    if( ( JalapenosLppCursor + LPP_PRESENCE_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_PRESENCE; 
    JalapenosLppBuffer[JalapenosLppCursor++] = value; 

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddTemperature( uint8_t channel, float celsius1, float celsius2, float celsius3 )
{
    if( ( JalapenosLppCursor + LPP_TEMPERATURE_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    union analogVal val;
		#if defined(HW_CF_SENSOR)
		val.analog =  ( int16_t )(celsius1*100);
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1];
		
		#elif defined(HW_ECOVETTE)
		val.analog =  ( int16_t )(celsius1*100);
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1];
		
		val.analog = ( int16_t )(celsius2*100);
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1];
		
		val.analog = ( int16_t )(celsius3*100);
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1];
		#endif
		
    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddRelativeHumidity( uint8_t channel, float rh1, float rh2, float rh3 )
{
    if( ( JalapenosLppCursor + LPP_RELATIVE_HUMIDITY_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    union analogVal val;
		val.analog = ( uint16_t )(rh1*100);
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1];

		
		val.analog = ( uint16_t )(rh2*100);
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1];
		
		val.analog = ( uint16_t )(rh3*100);;
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[0]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val.bytes[1];
		
    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddAccelerometer( uint8_t channel, float x, float y, float z )
{
    if( ( JalapenosLppCursor + LPP_ACCELEROMETER_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int16_t vx = ( int16_t ) ( x * 1000 );
    int16_t vy = ( int16_t ) ( y * 1000 );
    int16_t vz = ( int16_t ) ( z * 1000 );

    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_ACCELEROMETER; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vx >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vx; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vy >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vy; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vz >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vz; 

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddBarometricPressure( uint8_t channel, float hpa )
{
    if( ( JalapenosLppCursor + LPP_BAROMETRIC_PRESSURE_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int16_t val = ( int16_t ) ( hpa * 10 );

    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_BAROMETRIC_PRESSURE; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = val; 

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddGyrometer( uint8_t channel, float x, float y, float z )
{
    if( ( JalapenosLppCursor + LPP_GYROMETER_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int16_t vx = ( int16_t ) ( x * 100 );
    int16_t vy = ( int16_t ) ( y * 100 );
    int16_t vz = ( int16_t ) ( z * 100 );

    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_GYROMETER; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vx >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vx; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vy >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vy; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vz >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = vz; 

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddGps( uint8_t channel, float latitude, float longitude, float meters )
{
    if( ( JalapenosLppCursor + LPP_GPS_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int32_t lat = ( int32_t ) ( latitude * 10000 );
    int32_t lon = ( int32_t ) ( longitude * 10000 );
    int32_t alt = ( int32_t ) ( meters * 100 );

    JalapenosLppBuffer[JalapenosLppCursor++] = channel; 
    JalapenosLppBuffer[JalapenosLppCursor++] = LPP_GPS; 

    JalapenosLppBuffer[JalapenosLppCursor++] = lat >> 16; 
    JalapenosLppBuffer[JalapenosLppCursor++] = lat >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = lat; 
    JalapenosLppBuffer[JalapenosLppCursor++] = lon >> 16; 
    JalapenosLppBuffer[JalapenosLppCursor++] = lon >> 8; 
    JalapenosLppBuffer[JalapenosLppCursor++] = lon; 
    JalapenosLppBuffer[JalapenosLppCursor++] = alt >> 16; 
    JalapenosLppBuffer[JalapenosLppCursor++] = alt >> 8;
    JalapenosLppBuffer[JalapenosLppCursor++] = alt;

    return JalapenosLppCursor;
}

uint8_t JalapenosLppAddSpectral ( uint8_t channel, uint8_t *spectral_data){
	
		if( ( JalapenosLppCursor + LPP_SPECTRAL_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }

		for(uint8_t i = 0; i<24; i++)
		{
				JalapenosLppBuffer[JalapenosLppCursor++] = spectral_data[i];
		} 

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

uint8_t JalapenosLppAddTemperatureAndHumidity(int temp, int humid) {

	if(( JalapenosLppCursor + LPP_TEMP_AND_HUMID_SIZE) > Jalapenos_LPP_MAXBUFFER_SIZE ) {
        return 0;
    }
		
		union analogVal tempA;
		tempA.analog = temp;
		
		union analogVal humidA;
		humidA.analog = humid;
		
		JalapenosLppBuffer[JalapenosLppCursor++] = tempA.bytes[0];
		JalapenosLppBuffer[JalapenosLppCursor++] = tempA.bytes[1];
		
		JalapenosLppBuffer[JalapenosLppCursor++] = humidA.bytes[0];
    JalapenosLppBuffer[JalapenosLppCursor++] = humidA.bytes[1];
    return JalapenosLppCursor;	
}

uint8_t JalapenosLppAddPAM ( uint8_t channel, int minfluo, int maxfluo, int yield){

	if( ( JalapenosLppCursor + LPP_PAM_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    union analogVal y;
		y.analog = yield;		
		
    union analogVal f;
		f.analog = minfluo;
		
		union analogVal fm;
		fm.analog = maxfluo;

		JalapenosLppBuffer[JalapenosLppCursor++] = f.bytes[0];
		JalapenosLppBuffer[JalapenosLppCursor++] = f.bytes[1];

		
		JalapenosLppBuffer[JalapenosLppCursor++] = fm.bytes[0];
		JalapenosLppBuffer[JalapenosLppCursor++] = fm.bytes[1];


    JalapenosLppBuffer[JalapenosLppCursor++] = y.bytes[0]; 
    JalapenosLppBuffer[JalapenosLppCursor++] = y.bytes[1]; 

    return JalapenosLppCursor;	
}

uint8_t JalapenosLppAddDummy ( void ){
		if( ( JalapenosLppCursor + LPP_PAM_SIZE ) > Jalapenos_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
		
		int dummy1;
		int dummy2;
		
    union analogVal y;
		y.analog = dummy1;
		uint32_t deviceID = 34000;
		
		
		union analogVal ID;
		ID.analog = deviceID;
		
		
		JalapenosLppBuffer[JalapenosLppCursor++] = ID.bytes[0];
		JalapenosLppBuffer[JalapenosLppCursor++] = ID.bytes[1];
		JalapenosLppBuffer[JalapenosLppCursor++] = ID.bytes[2];

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