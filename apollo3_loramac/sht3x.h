/***************************************************************************//**
 * \file    sht3x.h
 *
 * \brief   Library to access the Sensirion SHT3x Temperature and Humidity
 *          sensor via I2C on Ambiq Apollo3.
 ******************************************************************************/

#ifndef SHT3X_H
#define SHT3X_H

#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"
#include "am_mcu_apollo.h"
#include "am_util_stdio.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/

#define ADDRESS_SHT3X                   0x69

// Commands sent as Instruction (ui32InstrLen=2), Apollo3 IOM sends LSB first
// so values are byte-swapped vs SHT3x datasheet
#define SHT3X_MEAS_HIGHREP              0x0024   // wire: [24][00], no clock stretching
#define SHT3X_MEAS_HIGHREP_CS           0x062C   // wire: [2C][06], clock stretching
#define SHT3X_SOFT_RESET                0xA230   // wire: [30][A2]

// CRC-8 parameters (as per SHT3x datasheet)
#define SHT3X_CRC_POLYNOMIAL            0x31
#define SHT3X_CRC_INIT                  0xFF

#define AM_DEVICES_SHT3X_MAX_DEVICE_NUM 1

/******************************************************************************
 * ERROR CODES
 *****************************************************************************/
typedef enum {
    SHT3X_NO_ERROR       = 0,
    SHT3X_ACK_ERROR      = 1,
    SHT3X_CHECKSUM_ERROR = 2,
} SHT3X_Error;

/******************************************************************************
 * EXTERNAL HANDLE
 *****************************************************************************/
extern void *my_IomdevHdl;

/******************************************************************************
 * FUNCTION PROTOTYPES
 *****************************************************************************/
SHT3X_Error SHT3X_GetTempAndHumi(float *temp, float *humi);
SHT3X_Error SHT3X_SoftReset(void);

#endif // SHT3X_H