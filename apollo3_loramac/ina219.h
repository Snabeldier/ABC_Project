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

/* Config: 32 V bus range, PGA=1 (+-40 mV shunt range), 12-bit ADC, continuous */
#define INA219_CONFIG_VALUE         0x219F

/* Shunt resistor on this board is 0.12 Ohm (measured; nominal marking "R100" = 0.1 Ohm).
 * Current_LSB is chosen to be 100 uA per LSB:
 * Cal = trunc(0.04096 / (Current_LSB * R)) = trunc(0.04096 / (100e-6 * 0.12)) = 3413.
 * Max measurable current at PGA=1: 40 mV / 0.12 Ohm = 333 mA (sufficient for LoRa TX peak ~46 mA). */
#define INA219_CALIBRATION          3413

/* Conversion factors matching the above configuration */
#define INA219_SHUNT_LSB_UV         10.0f   /* 10 uV per LSB (PGA=1, +-40 mV range) */
#define INA219_BUS_LSB_MV           4.0f    /* 4 mV per LSB */
#define INA219_CURRENT_LSB_UA       100.0f  /* 100 uA per LSB (Cal=3413, R=0.12 Ohm) */

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
