/***************************************************************************//**
 * \file    sht3x.h
 *
 * \brief   Driver for Sensirion SHT3x temperature and humidity sensor via I2C
 *          on the Ambiq Apollo3.
 ******************************************************************************/

#ifndef SHT3X_H
#define SHT3X_H

#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"
#include "am_mcu_apollo.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/

#define ADDRESS_SHT3X           0x44  /* ADDR pin = GND */

/* Command bytes as stored for Apollo3 IOM (sends instruction LSB-first).
 * SHT3x datasheet wire order is MSB-first, so values are byte-swapped here.
 * Example: datasheet command 0x2400 -> stored as 0x0024 -> wire: [0x24][0x00] */
#define SHT3X_CMD_MEAS_HIGHREP  0x0024   /* single-shot, high repeatability, no clock stretch */
#define SHT3X_CMD_SOFT_RESET    0xA230   /* soft reset */

#define SHT3X_CRC_POLYNOMIAL    0x31
#define SHT3X_CRC_INIT          0xFF

/******************************************************************************
 * TYPES
 *****************************************************************************/

typedef enum {
    SHT3X_NO_ERROR       = 0,
    SHT3X_ACK_ERROR      = 1,
    SHT3X_CHECKSUM_ERROR = 2,
} SHT3X_Error;

/******************************************************************************
 * EXTERNAL VARIABLES
 *****************************************************************************/

extern void *my_IomdevHdl;

/******************************************************************************
 * FUNCTION PROTOTYPES
 *****************************************************************************/

SHT3X_Error SHT3X_GetTempAndHumi(float *temp, float *humi);
SHT3X_Error SHT3X_GetRaw(uint16_t *temp_raw, uint16_t *hum_raw);
SHT3X_Error SHT3X_SoftReset(void);

#endif /* SHT3X_H */
