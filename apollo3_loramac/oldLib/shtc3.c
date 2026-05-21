/***************************************************************************//**
 * \file    shtc3.c
 *
 * \author  Johannes Klueppel  ( Uni Freiburg IMTEK )
 *
 * \mod		Timm Luhmann (IMTEK)
 *
 * \date    22.06.2022
 *
 * \brief   This is a small library to access the Sensirion SHTC3 Temperature and Humidity sensor via I2C.
 *
 * \todo    fix sleep and wake up
 *
 * \note    Big parts of this code are taken from the SHTC3 Sample Code (V1.0) from Sensirion AG, Switzerland
 ******************************************************************************/

#include "shtc3.h"

/******************************************************************************
 * VARIABLES
 *****************************************************************************/
static uint8_t _Address;

typedef struct
{
    uint32_t                    ui32Module;
    void                        *pIomHandle;
    bool                        bOccupied;
} am_devices_iom_shtc3_t;

am_devices_iom_shtc3_t gShtc3[AM_DEVICES_SHTC3_MAX_DEVICE_NUM];	//one device maximum


/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/
static etError SHTC3_ReadBytesAndCrc(uint16_t *data1, uint16_t *data2);
static etError SHTC3_CheckCrc(uint8_t data[], uint8_t nbrOfBytes,
                              uint8_t checksum);
static float SHTC3_CalcTemperature(uint16_t rawValue);
static float SHTC3_CalcHumidity(uint16_t rawValue);

/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/

//------------------------------------------------------------------------------
static etError SHTC3_ReadBytesAndCrc(uint16_t *data1, uint16_t *data2)
{
    etError error;    // error code
    uint8_t bytes[2];
    uint8_t checksum; // checksum byte
	uint32_t receive[2];


//  // read two data bytes and one checksum byte
//  bytes[0] = I2c_ReadByte(ACK);
//  bytes[1] = I2c_ReadByte(ACK);
//  checksum = I2c_ReadByte(ACK);

	am_devices_iom_shtc3_t *pIom = (am_devices_iom_shtc3_t *)my_IomdevHdl;

    //
    // Send the command sequence to read the Device ID.
    //
    if (am_device_command_read(pIom->pIomHandle, ADDRESS_SHTC3, 0,
                           0,
                           false, receive, 6))
    {
        return ACK_ERROR;
    }
	

    // verify checksum
    bytes[0] = receive[0];
    bytes[1] = receive[0]>>8;
    checksum = receive[0]>>16;
    error = SHTC3_CheckCrc(bytes, 2, checksum);
    // combine the two bytes to a 16-bit value
    *data1 = (bytes[0] << 8) | bytes[1];

    // verify checksum
    bytes[0] = receive[0]>>24;
    bytes[1] = receive[1];
    checksum = receive[1]>>8;
    error |= SHTC3_CheckCrc(bytes, 2, checksum);
    // combine the two bytes to a 16-bit value
    *data2 = (bytes[0] << 8) | bytes[1];

    return error;
}

//------------------------------------------------------------------------------
static etError SHTC3_CheckCrc(uint8_t data[], uint8_t nbrOfBytes,
                              uint8_t checksum)
{
    uint8_t bit;        // bit mask
    uint8_t crc = 0xFF; // calculated checksum
    uint8_t byteCtr;    // byte counter

    // calculates 8-Bit checksum with given polynomial
    for (byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
    {
        crc ^= (data[byteCtr]);
        for (bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ CRC_POLYNOMIAL;
            }
            else
            {
                crc = (crc << 1);
            }
        }
    }

    // verify checksum
    if (crc != checksum)
    {
        return CHECKSUM_ERROR;
    }
    else
    {
        return NO_ERROR;
    }
}

//------------------------------------------------------------------------------
static float SHTC3_CalcTemperature(uint16_t rawValue)
{
    // calculate temperature [°C]
    // T = -45 + 175 * rawValue / 2^16
    return 175 * (float) rawValue / 65536.0f - 45.0f;
}

//------------------------------------------------------------------------------
static float SHTC3_CalcHumidity(uint16_t rawValue)
{
    // calculate relative humidity [%RH]
    // RH = rawValue / 2^16 * 100
    return 100 * (float) rawValue / 65536.0f;
}

/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/

/***************************************************************************//**
 * \brief   Initializes the I2C bus for communication with the sensor.
 *
 * \param   [in]    address I2C address of the sensor IC. (see datasheet p.6)
 *
 * \retval          error   ACK_ERROR      = no acknowledgment from sensor
 *                          CHECKSUM_ERROR = checksum mismatch
 *                          NO_ERROR       = no error
 * \deprecated 		Not in use/required on ambiq apollo 3
 ******************************************************************************/
//etError SHTC3_Init(uint8_t address)
//{
//    _Address = address;
//    initI2C(_Address); // init I2C
//    etError error = NO_ERROR;
////    error = SHTC3_Sleep();
//    return error;
//}

/***************************************************************************//**
 * \brief   Gets the temperature [°C] and the humidity [%RH].
 *
 * \param   [out]   temp    pointer to a floating point value, where the calculated temperature will be stored
 * \param   [out]   humi    pointer to a floating point value, where the calculated humidity will be stored
 *
 * \retval          error   ACK_ERROR      = no acknowledgment from sensor
 *                          CHECKSUM_ERROR = checksum mismatch
 *                          NO_ERROR       = no error
 ******************************************************************************/
etError SHTC3_GetTempAndHumi(float *temp, float *humi)
{
    uint32_t command;
    uint16_t rawTemp;
    uint16_t rawHum;
    etError error = NO_ERROR;

//    error = SHTC3_Wakeup();
	timerDelay(1);		//give sensor time to wake up to prevent measurement errors
    // Set sensor in measureing mode (Temperature first, than Humidity)
    command = MEAS_T_RH_POLLING;
	am_devices_iom_shtc3_t *pIom = (am_devices_iom_shtc3_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_SHTC3, 2,
					   command,
					   false, 0, 0))
    {
        error |= ACK_ERROR;
    }

    // wait until measurement finished
    //waitInLPMzero(15);
	//this is very unoptimal, however an easy fix to wait..
	//am_hal_flash_delay(FLASH_CYCLES_US(15000));
	timerDelay(15);

    error |= SHTC3_ReadBytesAndCrc(&rawTemp, &rawHum);

    // if no error, calculate temperature in °C and humidity in %RH
    if (error == NO_ERROR)
    {
        *temp = SHTC3_CalcTemperature(rawTemp);
        *humi = SHTC3_CalcHumidity(rawHum);
    }

    return error;

}

/***************************************************************************//**
 * \brief   Gets the ID from the sensor.
 *
 * \param   [out]   id      pointer to a integer, where the id will be stored
 *
 * \retval          error   ACK_ERROR      = no acknowledgment from sensor
 *                          CHECKSUM_ERROR = checksum mismatch
 *                          NO_ERROR       = no error
 ******************************************************************************/
etError SHTC3_GetId(uint16_t *id)
{
    etError error = NO_ERROR; // error code
    uint8_t bytes[2];
    uint8_t checksum; // checksum byte
    uint32_t command;
	uint32_t receive;

    //
    command = READ_ID;
	am_devices_iom_shtc3_t *pIom = (am_devices_iom_shtc3_t *)my_IomdevHdl;

    //
    // Send the command sequence to read the Device ID.
    //
    if (am_device_command_read(pIom->pIomHandle, ADDRESS_SHTC3, 2,
                           command,
                           false, &receive, 3))
    {
        return ACK_ERROR;
    }

    if( error == NO_ERROR )
    {
        //i2creceive(_Address, 3);

        // verify checksum
        bytes[0] = receive;
        bytes[1] = receive>>8;
        checksum = receive>>16;
        error = SHTC3_CheckCrc(bytes, 2, checksum);
        // combine the two bytes to a 16-bit value
        *id = receive;
    }

    return error;
}

/***************************************************************************//**
 * \brief   Let the sensor go to sleep mode.
 *
 * \todo    fix me
 *
 * \retval          error   ACK_ERROR      = no acknowledgment from sensor
 *                          CHECKSUM_ERROR = checksum mismatch
 *                          NO_ERROR       = no error
 ******************************************************************************/
etError SHTC3_Sleep(void)
{
    uint32_t command;
    etError error = NO_ERROR;

    // set sensor in sleep mode
	command = SLEEP;
	am_devices_iom_shtc3_t *pIom = (am_devices_iom_shtc3_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_SHTC3, 2,
					   command,
					   false, 0, 0))
    {
        error |= ACK_ERROR;
    }
    return error;
}

/***************************************************************************//**
 * \brief   Wakes up the sensor from sleep mode.
 *
 * \todo    fix me
 *
 * \retval          error   ACK_ERROR      = no acknowledgment from sensor
 *                          CHECKSUM_ERROR = checksum mismatch
 *                          NO_ERROR       = no error
 ******************************************************************************/
etError SHTC3_Wakeup(void)
{
    uint32_t command;
    etError error = NO_ERROR;

    // set sensor in wakeup mode
	command = WAKEUP;
	am_devices_iom_shtc3_t *pIom = (am_devices_iom_shtc3_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_SHTC3, 2,
					   command,
					   false, 0, 0))
    {
        error |= ACK_ERROR;
    }

    // wait until woke up
	am_hal_flash_delay(FLASH_CYCLES_US(1000));

    return error;
}
