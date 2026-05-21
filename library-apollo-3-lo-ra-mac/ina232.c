/*!
 *  \file       ina232.c
 *
 *  \brief      Provides functions to use the external ina232 powermeter IC
 *
 *  \date       08.04.2024
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
//***** Header Files **********************************************************
#include <stdint.h>
//#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "pins.h"
#include "i2c.h"
#include "gpio.h"
#include "pins.h"
#include "am_util_stdio.h"
#include "ina232.h"
#include "dac63002.h"
#include "timing.h"

typedef struct
{
    uint32_t                    ui32Module;
    void                        *pIomHandle;
    bool                        bOccupied;
} am_devices_iom_ina232_t;

static uint8_t adcRange = 1;
//static float current_lsb = 10.72998046875e-6; // Value between CURRENT_LSB (minimum) and 8 x CURRENT_LSB(minimum), datasheet p. 26
static float current_lsb = 915.52734375e-9; // Value between CURRENT_LSB (minimum) and 8 x CURRENT_LSB(minimum), datasheet p. 26
static uint16_t shunt_cal = (uint16_t)(0.00512 / (CURRENT_LSB * 0.05)); // internal fixed value / ( _current_lsb * R in ohm ) = 9543

//*****************************************************************************
//
//! @brief Reads the ID of the INA and returns the value.
//!
//!
//! This function reads the device ID register of the INA232, and returns
//! true if the read value is correct
//
//*****************************************************************************
uint32_t
ina232_testCommunication(){
	uint32_t command = INA_232_ID_ADR;		// ID register
	am_devices_iom_ina232_t *pIom = (am_devices_iom_ina232_t *)my_IomdevHdl;
	uint32_t receive = 0;
    //
    // Send the command sequence to read the status
    //
    if (am_device_command_read(pIom->pIomHandle, INA_232_SLAVE_ID, 1,
                           command,
                           false, &receive, 2))
    {
        return ACKNOLEDGE_ERROR;
    }
	receive = dataPrepare(receive);
    if(receive == 0x5449){
        return 1;
    }
    return 0;
}
//*****************************************************************************
//
//! @brief Initializes the INA232 for regular operation
//!
//!
//! Sofar the default values for operation are chosen, ADC range is set via variable
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t ina232_init(){
	am_devices_iom_ina232_t *pIom = (am_devices_iom_ina232_t *)my_IomdevHdl;
	
	uint32_t dataByte = MODE_BUS_SHUNT_CONTINOUS + VSHCT_1100 + VBUSCT_1100 + AVG_1;		//default sofar
	
	if(adcRange == 0){
		dataByte += ADCRANGE_80;
	}
	else if(adcRange == 1){
		dataByte += ADCRANGE_20;
	}

	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, INA_232_SLAVE_ID, 1,
                           INA_232_CONFIG_REG,
                           false, &dataByte, 2))
    {
        return ACKNOLEDGE_ERROR;
    }

	if(adcRange == 0){
		dataByte = SHUNT_CAL_80;
	}
	else if(adcRange == 1){
		dataByte = SHUNT_CAL_20;
	}
	
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, INA_232_SLAVE_ID, 1,
                           INA_232_CALIBRATION_REG,
                           false, &dataByte, 2))
    {
        return ACKNOLEDGE_ERROR;
    }
    //
    // Return the status.
    //	
    return INASUCCESS;
}

//*****************************************************************************
//
//! @brief Read the shunt voltage using the ina232
//!
//!
//! Value is retrieved in 2's complement and therefore is converted before storing
//!
//!	@param *shuntVoltage:	pointer that is used to retrieve the shunt voltage as a floating point number
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t ina232_readShuntVoltage(float *shuntVoltage){
	am_devices_iom_ina232_t *pIom = (am_devices_iom_ina232_t *)my_IomdevHdl;
	uint32_t receive = 0;
    //
    // Send the command sequence to read the status
    //
	uint32_t mask = ina232_readRegister(INA_232_MASK_ENABLE_REG);
	if(mask & 0x08){
		timerDelay(5);
	}
    if (am_device_command_read(pIom->pIomHandle, INA_232_SLAVE_ID, 1,
                           INA_232_SHUNT_VOLTAGE_REG,
                           false, &receive, 2))
    {
        return ACKNOLEDGE_ERROR;
    }
	receive = dataPrepare(receive);
	//transform 2s complement into a float number
	if(receive & 0x80){
		receive -=1;	//substract 1
		receive = receive^0xFF;		//invert all bits
	}
	if(adcRange == 0){
		*shuntVoltage = receive * 2.5e-6;
	}
	else if(adcRange == 1){
		*shuntVoltage = receive * 625e-9;
	}
	return INASUCCESS;
}

//*****************************************************************************
//
//! @brief Read the bus voltage using the ina232
//!
//!
//!	@param *busVoltage:	pointer that is used to retrieve the bus voltage as a floating point number
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t ina232_readBusVoltage(float *busVoltage){
	am_devices_iom_ina232_t *pIom = (am_devices_iom_ina232_t *)my_IomdevHdl;
	uint32_t receive = 0;
    //
    // Send the command sequence to read the status
    //
	
	uint32_t mask = ina232_readRegister(INA_232_MASK_ENABLE_REG);
	if(mask & 0x08){
		timerDelay(5);
	}
    if (am_device_command_read(pIom->pIomHandle, INA_232_SLAVE_ID, 1,
                           INA_232_BUS_VOLTAGE_REG,
                           false, &receive, 2))
    {
        return ACKNOLEDGE_ERROR;
    }
	receive = dataPrepare(receive);
	*busVoltage = receive * 1.6e-3;
	return INASUCCESS;
}

//*****************************************************************************
//
//! @brief Read the power using the ina232
//!
//!
//!	@param *power: pointer that is used to retrieve the bus voltage as a floating point number
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t ina232_readPower(float *power){
	am_devices_iom_ina232_t *pIom = (am_devices_iom_ina232_t *)my_IomdevHdl;
	uint32_t receive = 0;
    //
    // Send the command sequence to read the status
    //
	uint32_t mask = ina232_readRegister(INA_232_MASK_ENABLE_REG);
	if(mask & 0x08){
		timerDelay(5);
	}
    if (am_device_command_read(pIom->pIomHandle, INA_232_SLAVE_ID, 1,
                           INA_232_POWER_REG,
                           false, &receive, 2))
    {
        return ACKNOLEDGE_ERROR;
    }
	receive = dataPrepare(receive);
	*power = 32 * CURRENT_LSB * receive;	//datasheet p. 24
	return INASUCCESS;
}

//*****************************************************************************
//
//! @brief Read the current using the ina232
//!
//!
//! Value is retrieved in 2's complement and therefore is converted before storing
//!
//!	@param *current:	pointer that is used to retrieve the shunt voltage as a floating point number
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t ina232_readCurrent(float *current){
	am_devices_iom_ina232_t *pIom = (am_devices_iom_ina232_t *)my_IomdevHdl;
	uint32_t receive = 0;
    //
    // Send the command sequence to read the status
    //
	uint32_t mask = ina232_readRegister(INA_232_MASK_ENABLE_REG);
	if(mask & 0x08){
		timerDelay(5);
	}
    if (am_device_command_read(pIom->pIomHandle, INA_232_SLAVE_ID, 1,
                           INA_232_CURRENT_REG,
                           false, &receive, 2))
    {
        return ACKNOLEDGE_ERROR;
    }
	receive = dataPrepare(receive);
	if(receive & 0x80){
		//transform 2s complement into a float number
		receive -=1;	//substract 1
		receive = receive^0xFF;		//invert all bits
	}
	*current = receive * CURRENT_LSB;
	return INASUCCESS;
}

uint32_t ina232_readRegister(uint32_t reg){
	am_devices_iom_ina232_t *pIom = (am_devices_iom_ina232_t *)my_IomdevHdl;
	uint32_t receive = 0;
    //
    // Send the command sequence to read the status
    //
    if (am_device_command_read(pIom->pIomHandle, INA_232_SLAVE_ID, 1,
                           reg,
                           false, &receive, 2))
    {
        return ACKNOLEDGE_ERROR;
    }
	receive = dataPrepare(receive);
	return receive;
}