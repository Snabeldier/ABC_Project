/*!
 *  \file       as7341.h
 *
 *  \date       02.04.2020
 *
 *  \author     daniel
 */
 
 #include "stdint.h"
 #include "stdbool.h"

#ifndef SENSORS_AS7341_H_
#define SENSORS_AS7341_H_

#define AM_DEVICES_AS7341_MAX_DEVICE_NUM 1

typedef enum
{
    SUCCESS,
    ERROR,
	ACKNO_ERROR
} as7341_status;

#define ID_AS7341_DEC 36
#define ADDRESS_AS7341  0x39
#define ID_REG_ADR_AS7341 0x92
#define STATUS_REG_ADR_AS7341 0x01
#define AS7341_REGADDR_ENABLE 0x80

//spectral gain settings
#define AS7341_AGAIN_05  0x00
#define AS7341_AGAIN_1   0x01
#define AS7341_AGAIN_2   0x02
#define AS7341_AGAIN_4   0x03
#define AS7341_AGAIN_8   0x04
#define AS7341_AGAIN_16  0x05
#define AS7341_AGAIN_32  0x06
#define AS7341_AGAIN_64  0x07
#define AS7341_AGAIN_128 0x08
#define AS7341_AGAIN_256 0x09       // default value
#define AS7341_AGAIN_512 0x0A


//Predefined sets for SMUX mapping
#define AS7341_SMUX_F1_F6        0x00

#define AS7341_SMUX_F7F8CDNF    0x01
#define AS7341_SMUX_F2F3F5F7F8C    0x02
#define AS7341_SMUX_F7F8        0x03
#define AS7341_SMUX_CDNF       0x04

extern uint16_t integration_time;
extern uint16_t spectralData[12];          //Why 12? -- Dark is also there
//Used to determine the position of the result in spectralData-Array
enum chpos {
            as_none=-1,
            as_f1=0,
            as_f2=1,
            as_f3=2,
            as_f4=3,
            as_f5=4,
            as_f6=5,
            as_f7=6,
            as_f8=7,
            as_clear=8,
            as_dark=9,
            as_nir=10,
            as_flicker=11

};
extern int8_t adcChMap[6];//={as_none,as_none,as_none,as_none,as_none,as_none}; //Internal - Maps the SMUX configuration (ADC0 - ADC5 to the corresponding channels back (CH0 - CH8,  Clear, NIR, Flicker)

extern unsigned int spectralDataOld[6];
extern bool firstChannels;                 // if true -> F1-F6 is measured.


as7341_status as7341_readStatusRegister(uint32_t *reg);
as7341_status as7341_writeSMUXmapping(uint8_t listId);   // mapping of channels to ADCs.
as7341_status as7341_startMeasurement(bool flicker, bool wait);
as7341_status as7341_setWaitTime(uint8_t wtime);
as7341_status as7341_setIntegrationTime();       // ToDo: use arguments to set integration time easily.
as7341_status as7341_setASTEP(int i);            // int i is a index here.
as7341_status as7341_setATIME(uint8_t i);
as7341_status as7341_setGain(uint8_t gain);
float as7341_getSpectralGain();
as7341_status as7341_transmitMeasurements();     // Transmits readings from AS back to MSP and updates spectralData array, channels are selected through the writeSMUXmapping
as7341_status as7341_readMeasurements();         // DEPRECADET ToDo: use arguments to select different channels to read.
//void as7341_readMeasurementsOld();
as7341_status as7341_enable();
void as7341_config();
as7341_status as7341_stopMeasuring();
void as7341_takeResponsivityIntoAccount();        // calculates basic counts/ responsivity with respect to F8 (F8 = 1). (only done for F1-F8)
void as7341_normalize();                // ToDo: normalize function.
unsigned int as7341_testCommunication(); // returns wether sensor responses, 1 if communication to sensor sucessful, 0 else

#endif /* SENSORS_AS7341_H_ */
