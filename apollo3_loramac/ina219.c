/***************************************************************************//**
 * \file    ina219.c
 *
 * \brief   Driver for Texas Instruments INA219 current/power monitor via I2C
 *          on the Ambiq Apollo3.
 *
 * \note    Measurement sequence:
 *          1. Call INA219_Init() once to write config and calibration registers.
 *          2. The INA219 runs in continuous mode; conversions complete in ~532 us.
 *          3. Call INA219_GetMeasurements() to read shunt voltage, bus voltage,
 *             and current.
 *
 * \note    Shunt resistor: 0.12 Ohm (measured; nominal marking "R100" = 0.1 Ohm).
 *          Max measurable current: +-333 mA. Current LSB = 100 uA. Bus voltage range: 0-32 V.
 ******************************************************************************/

#include "ina219.h"
#include "am_util_delay.h"

/******************************************************************************
 * TYPES
 *****************************************************************************/

typedef struct {
    uint32_t  ui32Module;
    void     *pIomHandle;
    bool      bOccupied;
} am_devices_iom_ina219_t;

/******************************************************************************
 * LOCAL FUNCTIONS
 *****************************************************************************/

static INA219_Error INA219_WriteRegister(uint8_t reg, uint16_t value)
{
    am_devices_iom_ina219_t *pIom = (am_devices_iom_ina219_t *)my_IomdevHdl;
    /* Apollo3 IOM packs data bytes little-endian (byte0 = bits[7:0] sent first).
     * INA219 expects MSB first on the wire, so swap bytes in the uint32_t word. */
    uint32_t data = (uint32_t)(value >> 8) | ((uint32_t)(value & 0xFF) << 8);
    if (am_device_command_write(pIom->pIomHandle, ADDRESS_INA219,
                                1, reg,
                                false, &data, 2))
    {
        return INA219_ACK_ERROR;
    }
    return INA219_NO_ERROR;
}

static INA219_Error INA219_ReadRegister(uint8_t reg, uint16_t *value)
{
    am_devices_iom_ina219_t *pIom = (am_devices_iom_ina219_t *)my_IomdevHdl;
    uint32_t receive = 0;
    if (am_device_command_read(pIom->pIomHandle, ADDRESS_INA219,
                               1, reg,
                               false, &receive, 2))
    {
        return INA219_ACK_ERROR;
    }
    /* Apollo3 packs received bytes little-endian: bits[7:0] = first byte received.
     * INA219 sends MSB first, so MSB lands in bits[7:0] — swap to reconstruct. */
    *value = (uint16_t)((receive & 0xFF) << 8) | (uint16_t)((receive >> 8) & 0xFF);
    return INA219_NO_ERROR;
}

/******************************************************************************
 * PUBLIC FUNCTIONS
 *****************************************************************************/

INA219_Error INA219_Init(void)
{
    if (INA219_WriteRegister(INA219_REG_CONFIG, INA219_CONFIG_VALUE) != INA219_NO_ERROR)
        return INA219_ACK_ERROR;
    if (INA219_WriteRegister(INA219_REG_CALIBRATION, INA219_CALIBRATION) != INA219_NO_ERROR)
        return INA219_ACK_ERROR;
    am_util_delay_ms(1);
    return INA219_NO_ERROR;
}

INA219_Error INA219_GetMeasurements(float *shuntVoltage_mV, float *busVoltage_V, float *current_uA)
{
    uint16_t raw;

    if (INA219_ReadRegister(INA219_REG_SHUNTVOLTAGE, &raw) != INA219_NO_ERROR)
        return INA219_ACK_ERROR;
    *shuntVoltage_mV = (float)(int16_t)raw * INA219_SHUNT_LSB_UV / 1000.0f;

    if (INA219_ReadRegister(INA219_REG_BUSVOLTAGE, &raw) != INA219_NO_ERROR)
        return INA219_ACK_ERROR;
    /* Bus voltage result is in bits [15:3]; bits [2:1] are CNVR and OVF flags */
    *busVoltage_V = (float)(raw >> 3) * INA219_BUS_LSB_MV / 1000.0f;

    if (INA219_ReadRegister(INA219_REG_CURRENT, &raw) != INA219_NO_ERROR)
        return INA219_ACK_ERROR;
    *current_uA = (float)(int16_t)raw * INA219_CURRENT_LSB_UA;

    return INA219_NO_ERROR;
}
