/***************************************************************************//**
 * \file    sht3x.c
 *
 * \brief   Library to access the Sensirion SHT3x Temperature and Humidity
 *          sensor via I2C on Ambiq Apollo3.
 *
 * \note    SHT3x communication protocol (single shot mode):
 *          1. Write 2-byte measurement command as Instruction (InstrLen=2)
 *          2. Wait 100ms
 *          3. Read 2 Words (8 bytes) from sensor
 ******************************************************************************/

#include "sht3x.h"

/******************************************************************************
 * TYPES
 *****************************************************************************/
typedef struct
{
    uint32_t    ui32Module;
    void        *pIomHandle;
    bool        bOccupied;
} am_devices_iom_sht3x_t;

am_devices_iom_sht3x_t gSht3x[AM_DEVICES_SHT3X_MAX_DEVICE_NUM];

/******************************************************************************
 * LOCAL FUNCTIONS
 *****************************************************************************/

//------------------------------------------------------------------------------
// CRC-8 (polynomial 0x31, init 0xFF) — as per SHT3x datasheet
// Verified: CRC([BE,EF]) = 0x92 ?
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// T [°C] = -45 + 175 * rawValue / (2^16 - 1)
//------------------------------------------------------------------------------
static float SHT3X_CalcTemperature(uint16_t rawValue)
{
    return 175.0f * (float)rawValue / 65535.0f - 45.0f;
}

//------------------------------------------------------------------------------
// RH [%] = 100 * rawValue / (2^16 - 1)
//------------------------------------------------------------------------------
static float SHT3X_CalcHumidity(uint16_t rawValue)
{
    return 100.0f * (float)rawValue / 65535.0f;
}

/******************************************************************************
 * PUBLIC FUNCTIONS
 *****************************************************************************/

//------------------------------------------------------------------------------
SHT3X_Error SHT3X_SoftReset(void)
{
    am_devices_iom_sht3x_t *pIom = (am_devices_iom_sht3x_t *)my_IomdevHdl;

    if (am_device_command_write(pIom->pIomHandle, ADDRESS_SHT3X,
                                2, SHT3X_SOFT_RESET,
                                false, 0, 0))
    {
        am_util_stdio_printf("[ERROR] SHT3X_SoftReset: ACK error\n");
        return SHT3X_ACK_ERROR;
    }

    am_hal_flash_delay(FLASH_CYCLES_US(2000));
    return SHT3X_NO_ERROR;
}

//------------------------------------------------------------------------------
SHT3X_Error SHT3X_GetTempAndHumi(float *temp, float *humi)
{
    SHT3X_Error error      = SHT3X_NO_ERROR;
    uint32_t    receive[2] = {0, 0};
    uint8_t     bytes[2];
    uint16_t    rawTemp;
    uint16_t    rawHumi;

    am_devices_iom_sht3x_t *pIom = (am_devices_iom_sht3x_t *)my_IomdevHdl;

    // --- Step 1: Send measurement command as Instruction ---
    if (am_device_command_write(pIom->pIomHandle, ADDRESS_SHT3X,
                                2, SHT3X_MEAS_HIGHREP,
                                false, 0, 0))
    {
        am_util_stdio_printf("[ERROR] SHT3X: Write measurement command failed\n");
        return SHT3X_ACK_ERROR;
    }

    // --- Step 2: Wait generously for measurement to complete ---
    am_hal_flash_delay(FLASH_CYCLES_US(100000)); // 100ms

    // --- Step 3: Read 2 words (8 bytes) ---
    if (am_device_command_read(pIom->pIomHandle, ADDRESS_SHT3X, 0,
                               0, false, receive, 6))
    {
        am_util_stdio_printf("[ERROR] SHT3X: Read failed\n");
        return SHT3X_ACK_ERROR;
    }

    am_util_stdio_printf("[DEBUG] Bytes: %02X %02X %02X %02X | %02X %02X %02X %02X\n",
        (uint8_t)(receive[0]),
        (uint8_t)(receive[0] >> 8),
        (uint8_t)(receive[0] >> 16),
        (uint8_t)(receive[0] >> 24),
        (uint8_t)(receive[1]),
        (uint8_t)(receive[1] >> 8),
        (uint8_t)(receive[1] >> 16),
        (uint8_t)(receive[1] >> 24));

    // --- Step 4: Extract bytes ---
    uint8_t T_MSB = (uint8_t)(receive[0]);
    uint8_t T_LSB = (uint8_t)(receive[0] >> 8);
    uint8_t CRC_T = (uint8_t)(receive[0] >> 16);
    uint8_t H_MSB = (uint8_t)(receive[0] >> 24);
    uint8_t H_LSB = (uint8_t)(receive[1]);
    uint8_t CRC_H = (uint8_t)(receive[1] >> 8);

    am_util_stdio_printf("[DEBUG] SHT3X Raw: T=%02X %02X CRC=%02X | H=%02X %02X CRC=%02X\n",
                         T_MSB, T_LSB, CRC_T, H_MSB, H_LSB, CRC_H);

    // --- Step 5: CRC check temperature ---
    bytes[0] = T_MSB;
    bytes[1] = T_LSB;
    if (SHT3X_CheckCrc(bytes, 2, CRC_T) != SHT3X_NO_ERROR)
    {
        am_util_stdio_printf("[ERROR] SHT3X: Temperature CRC mismatch\n");
        error |= SHT3X_CHECKSUM_ERROR;
    }
    rawTemp = ((uint16_t)T_MSB << 8) | T_LSB;

    // --- Step 6: CRC check humidity ---
    bytes[0] = H_MSB;
    bytes[1] = H_LSB;
    if (SHT3X_CheckCrc(bytes, 2, CRC_H) != SHT3X_NO_ERROR)
    {
        am_util_stdio_printf("[ERROR] SHT3X: Humidity CRC mismatch\n");
        error |= SHT3X_CHECKSUM_ERROR;
    }
    rawHumi = ((uint16_t)H_MSB << 8) | H_LSB;

    // --- Step 7: Convert to physical values ---
    if (error == SHT3X_NO_ERROR)
    {
        *temp = SHT3X_CalcTemperature(rawTemp);
        *humi = SHT3X_CalcHumidity(rawHumi);
        am_util_stdio_printf("[DEBUG] SHT3X: Temp=%.2f C, Humi=%.2f%%\n", *temp, *humi);
    }

    return error;
}