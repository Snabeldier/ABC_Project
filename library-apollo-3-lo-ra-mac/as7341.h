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

#define AS7341_CHIP_ID 			0x09

#define AS7341_ADDRESS  		0x39
#define AS7341_REG_ID 			0x92
#define AS7341_REG_ENABLE 	0x80
#define AS7341_REG_CFG1 		0xAA
#define AS7341_REG_STATUS 	0x95
#define AS7341_REG_STATUS2 	0xA3
#define AS7341_REG_ATIME 		0x81
#define AS7341_REG_ASTEP 		0xCA
#define AS7341_REG_WTIME		0x83
#define AS7341_REG_CFG6			0xAF

//spectral gain settings
typedef enum {
 AS7341_AGAIN_05,  
 AS7341_AGAIN_1,   
 AS7341_AGAIN_2,   
 AS7341_AGAIN_4,   
 AS7341_AGAIN_8,   
 AS7341_AGAIN_16,  
 AS7341_AGAIN_32,  
 AS7341_AGAIN_64,  
 AS7341_AGAIN_128, 
 AS7341_AGAIN_256,        // default value
 AS7341_AGAIN_512,
} as7341_gain_t; 


//Predefined sets for SMUX mapping
#define AS7341_SMUX_F1_F6        	0x00
#define AS7341_SMUX_F7F8CDNF    	0x01
#define AS7341_SMUX_F2F3F5F7F8C   0x02
#define AS7341_SMUX_F7F8        	0x03
#define AS7341_SMUX_CDNF       		0x04

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

as7341_status as7341_enable(void);
as7341_status as7341_testCommunication(void);
as7341_status as7341_startMeasurement(void);
as7341_status as7341_stopMeasuring(void);

as7341_status as7341_config(uint8_t atime, uint16_t astep, as7341_gain_t gain);
as7341_status as7341_setGain(as7341_gain_t gain);
as7341_status as7341_getGain(uint16_t *reg);
as7341_status as7341_setASTEP(uint16_t astep);
as7341_status as7341_getASTEP(uint32_t *reg);
as7341_status as7341_setATIME(uint8_t atime);
as7341_status as7341_getATIME(uint32_t *reg);
as7341_status as7341_setTINT(uint8_t atime, uint16_t astep);
as7341_status as7341_getTINT(double *reg);
as7341_status as7341_setWaitTime(uint8_t wtime);

as7341_status as7341_convertGain(uint16_t *gain);
as7341_status as7341_writeSMUXmapping(uint8_t listId);   // mapping of channels to ADCs.

as7341_status as7341_delayForData(void);
as7341_status as7341_delayForSMUX(void);

as7341_status as7341_transmitMeasurements(uint16_t *data);     // Transmits readings from AS back to MSP and updates spectralData array, channels are selected through the writeSMUXmapping

as7341_status as7341_readRegister(uint32_t reg, uint32_t *data);		//read a register

as7341_status as7341_setInterruptMode(void);
as7341_status as7341_clearInterrupt(uint8_t flags);
#endif /* SENSORS_AS7341_H_ */
