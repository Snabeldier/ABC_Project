/*!
 * \file      deviceConfig.c
 *
 * \brief     Per-node configuration table — the single place where all
 *            device-specific values of both sensor nodes are entered.
 *
 * See deviceConfig.h for how the selection works.
 */
#include "deviceConfig.h"

#include "am_mcu_apollo.h"
#include "am_util_stdio.h"

/*!
 * One entry per physical board. Entry 0 is the fallback if the running chip
 * matches no entry (a warning is printed in that case).
 *
 * TTN console: the session keys are shown under
 * End device -> Settings -> Activation (ABP, LoRaWAN 1.1: FNwkSIntKey,
 * SNwkSIntKey, NwkSEncKey, AppSKey).
 */
static const DeviceConfig_t DeviceConfigTable[] =
{
    { /* ---- Node 34000 (values previously hardcoded in se-identity.h) ---- */
        .ChipId0         = 0xCA232551,
        .PayloadDeviceId = 34000,
        .DevAddr         = ( uint32_t )0x260BAA7F,
        .DevEui          = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x06, 0x6F, 0xB8 },
        .FNwkSIntKey     = { 0xEC, 0x5E, 0x88, 0xB6, 0xDB, 0x32, 0x94, 0xAB, 0xB7, 0x64, 0xD1, 0x4D, 0x67, 0x69, 0xD5, 0xDF },
        .SNwkSIntKey     = { 0x53, 0x8F, 0xD5, 0x4A, 0x6C, 0x36, 0xBC, 0x6F, 0x19, 0xB5, 0x92, 0x77, 0x06, 0x65, 0xEC, 0x57 },
        .NwkSEncKey      = { 0x65, 0x80, 0x22, 0x1F, 0x60, 0xF8, 0xFD, 0xBE, 0xB2, 0x4E, 0x7A, 0xA6, 0xCC, 0xEF, 0x8B, 0x36 },
        .AppSKey         = { 0xBB, 0x42, 0x94, 0x89, 0xB9, 0x0B, 0xD8, 0x9D, 0x9A, 0x07, 0x4D, 0x5A, 0x1B, 0x11, 0xF1, 0x45 },
    },
    { /* ---- Node 34001 ---- */
        .ChipId0         = 0xCA23A551,
        .PayloadDeviceId = 34001,
        .DevAddr         = ( uint32_t )0x260B0412,
        .DevEui          = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x77, 0xC5 },
        .FNwkSIntKey     = { 0x9E, 0x1C, 0x03, 0x2F, 0xC6, 0x4E, 0x41, 0x39, 0xD5, 0x75, 0xFC, 0xAF, 0x15, 0x5C, 0xA9, 0x40 },
        .SNwkSIntKey     = { 0x4C, 0xAC, 0xDA, 0xF4, 0x02, 0x00, 0x6E, 0xFA, 0xF7, 0x7B, 0xEC, 0x57, 0x9E, 0x51, 0xE5, 0x2B }, /* TODO */
        .NwkSEncKey      = { 0xC8, 0x03, 0xEF, 0xE4, 0x87, 0x75, 0xF8, 0x81, 0xED, 0xDD, 0x1B, 0xAA, 0x9A, 0x1A, 0x13, 0xDA }, /* TODO */
        .AppSKey         = { 0x75, 0x3B, 0xB6, 0x71, 0x81, 0x9A, 0xDF, 0xF0, 0x64, 0xEA, 0xD9, 0x47, 0x26, 0xE9, 0xD7, 0xA0 }, /* TODO */
    },
};

#define DEVICE_CONFIG_TABLE_SIZE ( sizeof( DeviceConfigTable ) / sizeof( DeviceConfigTable[0] ) )

const DeviceConfig_t* DeviceConfigGet( void )
{
    static const DeviceConfig_t* selected = 0;

    if( selected == 0 )
    {
        am_hal_mcuctrl_device_t device;
        am_hal_mcuctrl_info_get( AM_HAL_MCUCTRL_INFO_DEVICEID, &device );

        for( uint32_t i = 0; i < DEVICE_CONFIG_TABLE_SIZE; i++ )
        {
            if( DeviceConfigTable[i].ChipId0 == device.ui32ChipID0 )
            {
                selected = &DeviceConfigTable[i];
                break;
            }
        }

        if( selected == 0 )
        {
            // Unknown board: fall back to the first entry so the node keeps
            // transmitting, but make the mismatch visible on the console.
            selected = &DeviceConfigTable[0];
            am_util_stdio_printf( "[WARNING] ChipID0 0x%08X not in DeviceConfigTable, using node %u config\n",
                                  device.ui32ChipID0, (unsigned int)selected->PayloadDeviceId );
        }
    }

    return selected;
}
