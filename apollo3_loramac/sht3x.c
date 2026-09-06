/***************************************************************************//**
 * \file    sht3x.c
 *
 * \brief   Driver for Sensirion SHT3x temperature and humidity sensor via I2C
 *          on the Ambiq Apollo3.
 *
 * \note    Single-shot measurement sequence (no clock stretching):
 *          1. Write 2-byte command SHT3X_CMD_MEAS_HIGHREP
 *          2. Wait >= 15 ms for conversion to complete
 *          3. Read 6 bytes: [T_MSB][T_LSB][CRC_T][H_MSB][H_LSB][CRC_H]
 ******************************************************************************/

#include "sht3x.h"
#include "am_util_delay.h"

/******************************************************************************
 * TYPES
 *****************************************************************************/

typedef struct {
    uint32_t  ui32Module;
    void     *pIomHandle;
    bool      bOccupied;
} am_devices_iom_sht3x_t;

/******************************************************************************
 * LOCAL FUNCTIONS
 *****************************************************************************/

static SHT3X_Error SHT3X_CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
{
    uint8_t crc = SHT3X_CRC_INIT;

    for (uint8_t byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
    {
        crc ^= data[byteCtr];
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ SHT3X_CRC_POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }

    return (crc == checksum) ? SHT3X_NO_ERROR : SHT3X_CHECKSUM_ERROR;
}

static float SHT3X_CalcTemperature(uint16_t rawValue)
{
    return 175.0f * (float)rawValue / 65535.0f - 45.0f;
}

static float SHT3X_CalcHumidity(uint16_t rawValue)
{
    float rh = 100.0f * (float)rawValue / 65535.0f;
    if (rh > 100.0f) rh = 100.0f;
    if (rh < 0.0f)   rh = 0.0f;
    return rh;
}

/* Shared I2C sequence used by both SHT3X_GetTempAndHumi and SHT3X_GetRaw. */
static SHT3X_Error SHT3X_ReadRaw(uint16_t *temp_raw, uint16_t *hum_raw)
{
    SHT3X_Error error = SHT3X_NO_ERROR;
    uint32_t    receive[2];
    uint8_t     bytes[2];
    uint8_t     checksum;

    am_devices_iom_sht3x_t *pIom = (am_devices_iom_sht3x_t *)my_IomdevHdl;

    uint32_t cmd_lsb = 0x00;
    if (am_device_command_write(pIom->pIomHandle, ADDRESS_SHT3X,
                                1, 0x24, false, &cmd_lsb, 1))
        return SHT3X_ACK_ERROR;

    am_util_delay_ms(20);

    if (am_device_command_read(pIom->pIomHandle, ADDRESS_SHT3X,
                               0, 0, false, receive, 6))
        return SHT3X_ACK_ERROR;

    bytes[0] = (uint8_t)(receive[0]);
    bytes[1] = (uint8_t)(receive[0] >> 8);
    checksum  = (uint8_t)(receive[0] >> 16);
    if (SHT3X_CheckCrc(bytes, 2, checksum) != SHT3X_NO_ERROR)
        error = SHT3X_CHECKSUM_ERROR;
    *temp_raw = ((uint16_t)bytes[0] << 8) | bytes[1];

    bytes[0] = (uint8_t)(receive[0] >> 24);
    bytes[1] = (uint8_t)(receive[1]);
    checksum  = (uint8_t)(receive[1] >> 8);
    if (SHT3X_CheckCrc(bytes, 2, checksum) != SHT3X_NO_ERROR)
        error = SHT3X_CHECKSUM_ERROR;
    *hum_raw = ((uint16_t)bytes[0] << 8) | bytes[1];

    return error;
}

/******************************************************************************
 * PUBLIC FUNCTIONS
 *****************************************************************************/

SHT3X_Error SHT3X_SoftReset(void)
{
    am_devices_iom_sht3x_t *pIom = (am_devices_iom_sht3x_t *)my_IomdevHdl;
    /* Soft reset: 2-byte command [0x30][0xA2] sent as 1-byte instr + 1 data byte */
    uint32_t lsb = 0xA2;
    if (am_device_command_write(pIom->pIomHandle, ADDRESS_SHT3X,
                                1, 0x30,
                                false, &lsb, 1))
    {
        return SHT3X_ACK_ERROR;
    }

    am_util_delay_ms(2);
    return SHT3X_NO_ERROR;
}

SHT3X_Error SHT3X_GetTempAndHumi(float *temp, float *humi)
{
    uint16_t    rawTemp, rawHumi;
    SHT3X_Error error = SHT3X_ReadRaw(&rawTemp, &rawHumi);
    if (error == SHT3X_NO_ERROR)
    {
        *temp = SHT3X_CalcTemperature(rawTemp);
        *humi = SHT3X_CalcHumidity(rawHumi);
    }
    return error;
}

SHT3X_Error SHT3X_GetRaw(uint16_t *temp_raw, uint16_t *hum_raw)
{
    return SHT3X_ReadRaw(temp_raw, hum_raw);
}
