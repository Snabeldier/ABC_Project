/*!
 * \file      deviceConfig.h
 *
 * \brief     Per-node configuration, selected at runtime via the Apollo3
 *            unique chip ID (MCUCTRL CHIPID0).
 *
 * Both sensor nodes run the identical firmware image. Everything that differs
 * between the two nodes (payload device ID, TTN DevAddr, DevEUI and the ABP
 * session keys) lives in the table in deviceConfig.c. At boot the firmware
 * reads CHIPID0 and picks the matching table entry; if no entry matches, the
 * first entry (node 34000) is used as fallback.
 *
 * To enrol a node: flash it, read the "[INFO] ChipID0 = 0x........" boot
 * message from the SWO/printf viewer and put that value into the node's
 * table entry in deviceConfig.c.
 */
#ifndef __DEVICE_CONFIG_H__
#define __DEVICE_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
    /*!
     * Apollo3 unique chip ID (MCUCTRL CHIPID0) identifying the physical board.
     * Printed at boot as "[INFO] ChipID0 = 0x........".
     */
    uint32_t ChipId0;

    /*!
     * Device ID transmitted in the JalapenosLpp payload (34000 / 34001)
     */
    uint32_t PayloadDeviceId;

    /*!
     * TTN device address (ABP, big endian, "Device address" in the TTN console)
     */
    uint32_t DevAddr;

    /*!
     * End-device IEEE EUI (big endian, "DevEUI" in the TTN console)
     */
    uint8_t DevEui[8];

    /*!
     * Forwarding network session integrity key
     * (TTN: "FNwkSIntKey"; for LoRaWAN 1.0.x this is the NwkSKey)
     */
    uint8_t FNwkSIntKey[16];

    /*!
     * Serving network session integrity key (TTN: "SNwkSIntKey")
     */
    uint8_t SNwkSIntKey[16];

    /*!
     * Network session encryption key (TTN: "NwkSEncKey")
     */
    uint8_t NwkSEncKey[16];

    /*!
     * Application session key (TTN: "AppSKey")
     */
    uint8_t AppSKey[16];
} DeviceConfig_t;

/*!
 * Returns the configuration of the node the firmware is currently running on.
 * The chip ID is read once on the first call, the result is cached.
 */
const DeviceConfig_t* DeviceConfigGet( void );

#ifdef __cplusplus
}
#endif

#endif // __DEVICE_CONFIG_H__
