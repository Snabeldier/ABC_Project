/*!
 *  \file       am1805.h
 *
 *  \brief      Provides functions to use the external ina232 powermeter IC
 *
 *  \date       08.04.2024
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
#ifndef INA232_H
#define INA232_H


#include "stdint.h"

typedef enum
{
    INASUCCESS,
    INAERROR,
	ACKNOLEDGE_ERROR
} ina232_status;

#define INA_232_SLAVE_ID          0x40

#define INA_232_CONFIG_REG				0x00
#define INA_232_SHUNT_VOLTAGE_REG 		0x01
#define INA_232_BUS_VOLTAGE_REG			0x02
#define INA_232_POWER_REG				0x03
#define INA_232_CURRENT_REG				0x04
#define INA_232_CALIBRATION_REG			0x05
#define INA_232_MASK_ENABLE_REG			0x06
#define INA_232_ALERT_LIMIT_REG			0x07
#define INA_232_ID_ADR					0x3E

//#define CURRENT_LSB 					10.72998046875e-6		//p. 24 datasheet, based on 6 KXOB25 solar cells
#define CURRENT_LSB						915.52734375e-9

#define SHUNT_CAL_80					9543		//0,00512/(Current_LSB*0,05)
//#define SHUNT_CAL_20					2386		//divided by 4
#define SHUNT_CAL_20					27962		//divided by 4

#define ADCRANGE_20						0x1000
#define ADCRANGE_80						0			//default

#define AVG_1							0			//default
#define AVG_4							0x200
#define AVG_16							0x400
#define AVG_64							0x600
#define AVG_128							0x800
#define AVG_256							0xA00
#define AVG_512							0xC00	
#define AVG_1024						0xE00

#define VBUSCT_140						0
#define VBUSCT_204						0x40
#define VBUSCT_332						0x80
#define VBUSCT_588						0xC0
#define VBUSCT_1100						0x100		//default
#define VBUSCT_2116						0x140
#define VBUSCT_4156						0x180
#define VBUSCT_8244						0x1C0

#define VSHCT_140						0
#define VSHCT_204						0x08
#define VSHCT_332						0x10
#define VSHCT_588						0x18
#define VSHCT_1100						0x20		//default
#define VSHCT_2116						0x28
#define VSHCT_4156						0x30
#define VSHCT_8244						0x38

#define MODE_SHUTDOWN					0
#define MODE_SHUNT_SINGLE				1
#define MODE_BUS_SINGLE					2
#define MODE_BUS_SHUNT_SINGLE			3
#define MODE_SHUTDOWN2					4
#define MODE_SHUNT_CONTINUOS			5
#define MODE_BUS_CONTINOUS				6
#define MODE_BUS_SHUNT_CONTINOUS		7			//default

uint32_t ina232_testCommunication();
uint32_t ina232_init();
uint32_t ina232_readShuntVoltage(float *shuntVoltage);
uint32_t ina232_readBusVoltage(float *busVoltage);
uint32_t ina232_readPower(float *power);
uint32_t ina232_readCurrent(float *current);
uint32_t ina232_readRegister(uint32_t reg);


#endif //INA232_H