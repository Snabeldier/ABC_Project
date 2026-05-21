/***************************************************************************//**
 * \file    shtc3.h
 *
 * \author  Johannes Klueppel  ( Uni Freiburg IMTEK )
 *
 * \mod		Timm Luhmann (IMTEK)
 *
 * \date    22.06.2022
 *
 * \brief   This is a small library to access the Sensirion SHTC3 Temperature and Humidity sensor via I2C.
 *
 * \note Big parts of this code are taken from the SHTC3 Sample Code (V1.0) from Sensirion AG, Switzerland
 ******************************************************************************/

#ifndef SENSORS_SHTC3_H_
#define SENSORS_SHTC3_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "stdint.h"

#include "i2c.h"
#include "timing.h"
#include "am_mcu_apollo.h"

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/
#define CRC_POLYNOMIAL  0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001
#define ADDRESS_SHTC3   0x70
#define AM_DEVICES_SHTC3_MAX_DEVICE_NUM 1

// Error codes
typedef enum{
  NO_ERROR       = 0x00,    // no error
  ACK_ERROR      = 0x01,    // no acknowledgment error
  CHECKSUM_ERROR = 0x02     // checksum mismatch error
}etError;

typedef enum
{
    READ_ID = 0xEFC8, // command: read ID register
    SOFT_RESET = 0x805D, // soft reset
    SLEEP = 0xB098, // sleep
    WAKEUP = 0x3517, // wakeup
    MEAS_T_RH_POLLING = 0x7866, // meas. read T first, clock stretching disabled
    MEAS_T_RH_CLOCKSTR = 0x7CA2, // meas. read T first, clock stretching enabled
    MEAS_RH_T_POLLING = 0x58E0, // meas. read RH first, clock stretching disabled
    MEAS_RH_T_CLOCKSTR = 0x5C24 // meas. read RH first, clock stretching enabled
} etCommands;


/******************************************************************************
 * VARIABLES
 *****************************************************************************/

/******************************************************************************
 * FUNCTION PROTOTYPES
 *****************************************************************************/
//etError SHTC3_Init(uint8_t address);
etError SHTC3_GetId(uint16_t *id);
etError SHTC3_GetTempAndHumi(float *temp, float *humi);

etError SHTC3_Sleep(void);
etError SHTC3_Wakeup(void);


#endif /* SENSORS_SHTC3_H_ */
