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

/* Calibration for 100 Ohm shunt, Current_LSB = 1 uA:
 * Cal = trunc(0.04096 / (1e-6 * 100)) = 409 */
#define INA219_CALIBRATION          409

/* Conversion factors matching the above configuration */
#define INA219_SHUNT_LSB_UV         10.0f   /* 10 uV per LSB (PGA=8, +-320 mV range) */
#define INA219_BUS_LSB_MV           4.0f    /* 4 mV per LSB */
#define INA219_CURRENT_LSB_UA       1.0f    /* 1 uA per LSB (with Cal=409) */

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
