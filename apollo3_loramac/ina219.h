/***************************************************************************//**
 * \file    ina219.h
 *
 * \brief   Driver for Texas Instruments INA219 current/power monitor via I2C
 *          on the Ambiq Apollo3.
 ******************************************************************************/

#ifndef INA219_H
#define INA219_H

#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"
#include "am_mcu_apollo.h"

/******************************************************************************
 * DEFINES
 *****************************************************************************/

#define ADDRESS_INA219              0x40

/* INA219 register addresses */
#define INA219_REG_CONFIG           0x00
#define INA219_REG_SHUNTVOLTAGE     0x01
#define INA219_REG_BUSVOLTAGE       0x02
#define INA219_REG_CURRENT          0x04
#define INA219_REG_CALIBRATION      0x05

/* Config: 32 V bus range, PGA=8 (+-320 mV shunt range), 12-bit ADC, continuous */
#define INA219_CONFIG_VALUE         0x399F

/* Shunt resistor on this board is 0.1 Ohm (marking "R100").
 * Current_LSB is chosen to match the shunt-ADC resolution:
 *   10 uV (shunt LSB) / 0.1 Ohm = 100 uA per current LSB.
 * Cal = trunc(0.04096 / (Current_LSB * R)) = trunc(0.04096 / (100e-6 * 0.1)) = 4096.
 * Max measurable current at PGA=8: 320 mV / 0.1 Ohm = 3.2 A (enough for LoRa TX peak). */
#define INA219_CALIBRATION          4096

/* Conversion factors matching the above configuration */
#define INA219_SHUNT_LSB_UV         10.0f   /* 10 uV per LSB (PGA=8, +-320 mV range) */
#define INA219_BUS_LSB_MV           4.0f    /* 4 mV per LSB */
#define INA219_CURRENT_LSB_UA       100.0f  /* 100 uA per LSB (Cal=4096, R=0.1 Ohm) */

/******************************************************************************
 * TYPES
 *****************************************************************************/

typedef enum {
    INA219_NO_ERROR  = 0,
    INA219_ACK_ERROR = 1,
} INA219_Error;

/******************************************************************************
 * EXTERNAL VARIABLES
 *****************************************************************************/

extern void *my_IomdevHdl;

/******************************************************************************
 * FUNCTION PROTOTYPES
 *****************************************************************************/

/* Writes config and calibration registers. Call once before first measurement. */
INA219_Error INA219_Init(void);

/* Reads shunt voltage (mV), bus voltage (V), and current (uA) in one sequence. */
INA219_Error INA219_GetMeasurements(float *shuntVoltage_mV, float *busVoltage_V, float *current_uA);

#endif /* INA219_H */
