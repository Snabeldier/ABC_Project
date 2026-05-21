#ifndef SHT4_H
#define SHT4_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void sht4_init(void *iom_handle, uint8_t i2c_address);
bool sht4_read_serial(uint32_t *serial_out);

bool sht4_read_temp_humidity(void *iom_handle, uint8_t i2c_address, float *temp_c, float *rh_percent);
typedef enum sht4x_status {
    SHT4X_SUCCESS = 0,
    SHT4X_ERROR,
    SHT4X_ACK_ERROR
} sht4x_status;

typedef enum {
    NO_ERROR_SHT4       = 0x00,  // no error
    ACK_ERROR_SHT4      = 0x01,  // I2C acknowledgment failure
    CHECKSUM_ERROR_SHT4 = 0x02   // CRC mismatch (not used here, but for completeness)
} sht4etError;



#define AM_DEVICES_SHT4X_MAX_DEVICE_NUM 1
#define SHT4X_I2C_ADDRESS 0x44  // Or 0x45 if using alternate address


sht4etError SHT4x_GetTempAndHumi(float temp[3], float humi[3]);

#ifdef __cplusplus
}
#endif

#endif // SHT4_H
