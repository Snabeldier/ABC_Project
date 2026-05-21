#include "i2c.h"
#include <stdint.h>
#include <stdbool.h>
#include "am_util_delay.h"
#include "sht4.h"
#include "am_util_stdio.h"

#define SHT4X_CMD_MEAS_HIGH      0xFD
#define SHT4X_CRC_POLYNOMIAL     0x31
#define SHT4X_CRC_INIT           0xFF

typedef struct {
    uint32_t ui32Module;
    void *pIomHandle;
    bool bOccupied;
} am_devices_iom_sht4x_t;

am_devices_iom_sht4x_t gSHT4x[AM_DEVICES_SHT4X_MAX_DEVICE_NUM];  // only one device supported

static uint8_t calculate_crc(uint8_t *data, uint8_t len)
{
    uint8_t crc = SHT4X_CRC_INIT;
    for (uint8_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; ++b) {
            crc = (crc & 0x80) ? ((crc << 1) ^ SHT4X_CRC_POLYNOMIAL) : (crc << 1);
        }
    }
    return crc;
}

bool sht4_read_temp_humidity(void *iom_handle, uint8_t i2c_address, float *temp_c, float *rh_percent)
{
    uint32_t command = SHT4X_CMD_MEAS_HIGH;

    // Send 1-byte command (0xFD)
    if (am_device_command_write(iom_handle, i2c_address, 1,
                                command, false, 0, 0)) {
        return false;
    }

    // Wait 10 ms (sensor measurement time)
    am_util_delay_ms(10);

    // Read 6 bytes: temp[2] + crc + rh[2] + crc
    uint32_t rxBuf[2] = {0};  // Enough for 6 bytes
    if (am_device_command_read(iom_handle, i2c_address, 0,
                               0, false, rxBuf, 6)) {
        return false;
    }

    uint8_t *bytes = (uint8_t *)rxBuf;

    if (calculate_crc(&bytes[0], 2) != bytes[2]) return false;
    if (calculate_crc(&bytes[3], 2) != bytes[5]) return false;

    uint16_t t_ticks  = (bytes[0] << 8) | bytes[1];
    uint16_t rh_ticks = (bytes[3] << 8) | bytes[4];

    float t  = -45.0f + 175.0f * ((float)t_ticks / 65535.0f);
    float rh = -6.0f + 125.0f * ((float)rh_ticks / 65535.0f);

    // Clamp humidity
    if (rh > 100.0f) rh = 100.0f;
    if (rh < 0.0f)   rh = 0.0f;

    *temp_c = t;
    *rh_percent = rh;

    return true;
}

sht4etError SHT4x_GetTempAndHumi(float temp[3], float humi[3])
{
    const uint8_t sensor_addresses[3] = { 0x44, 0x45, 0x46 };
    sht4etError error = NO_ERROR_SHT4;

    am_devices_iom_sht4x_t *pIom = (am_devices_iom_sht4x_t *)my_IomdevHdl;
		
		#if defined(HW_CF_SENSOR)
			uint8_t number_sensors = 1;
		#elif defined(HW_ECOVETTE)
			uint8_t number_sensors = 3;
		#endif

    for (int i = 0; i < number_sensors; ++i)
    {
        float t = 0.0f, h = 0.0f;

        bool success = sht4_read_temp_humidity(pIom->pIomHandle, sensor_addresses[i], &t, &h);

        if (success)
        {
            temp[i] = t;
            humi[i] = h;

            am_util_stdio_printf("SHT4x (0x%02X) - Temp: %.2f C, Hum: %.2f %%RH\n",
                                 sensor_addresses[i], t, h);
        }
        else
        {
            temp[i] = 0.0f;
            humi[i] = 0.0f;

            am_util_stdio_printf("SHT4x (0x%02X) - Read failed!\n", sensor_addresses[i]);
            error |= (ACK_ERROR_SHT4 << i);  // unique bit per sensor
        }
    }

    return error;
}



