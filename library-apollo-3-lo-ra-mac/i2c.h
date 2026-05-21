/*!
 *  \file       i2c.h
 *
 *  \brief      Provides functions to use the I2C driver
 *
 *  \date       02.02.2023
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
#ifndef I2C_H
#define I2C_H
#include "stdint.h"
#include "stdbool.h"



#define DEVICE_I2C_MAX_DEVICE_NUM 1

typedef enum
{
    DEVICE_I2C_STATUS_SUCCESS,
    DEVICE_I2C_STATUS_ERROR
} device_I2C_status_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} device_I2C_config_t;

extern void *my_pIomHandle;
extern void *my_IomdevHdl;
extern uint32_t I2Cbuffer[256];
//*****************************************************************************
//
//! @brief Initialize the I2C driver.
//!
//! @param ui32Module  - IOM module - for I2C always 1
//! @param pDMACtrlBuffer - DMA Transfer Control Buffer.
//! @deprecated settings are now done in the funtion to reduce complexity for now
//! This function should be called before any other am_devices_mb85rc64ta
//! functions. It is used to set tell the other functions how to communicate
//! with the external spiflash hardware.
//!
//! @return Status.
//
//*****************************************************************************
uint32_t
//device_I2C_init(uint32_t ui32Module, device_I2C_config_t *pDevConfig);
device_I2C_init(void);



//*****************************************************************************
//
//! @brief De-Initialize the I2C driver.
//!
//! @param *pHandle     - pointer on I2C handle
//!
//! This function reverses the initialization
//!
//! @return Status.
//
//*****************************************************************************
uint32_t
device_I2C_term(void);

//*****************************************************************************
//
// Generic Command Read function.
//
// @param *pHandle - Pointer on the handle used for I2C communication. Part of a struct that defines the communication module consisting of the handle, 
//
// @param ui8DevAddr - Device address
//
// @param ui32InstrLen - length of instruction
//
// @param ui64Instr - instruction/Command
//
// @param bCont hold the Bus for more instructions?
//
// @param *pData - pointer on the databuffer
//
// @param ui32NumBytes - number of bytes to receive
//*****************************************************************************
uint32_t am_device_command_read(void *pHandle, uint8_t ui8DevAddr, uint32_t ui32InstrLen, uint64_t ui64Instr,
                       bool bCont, uint32_t *pData, uint32_t ui32NumBytes);



//*****************************************************************************
//
// Generic Command Write function.
//
// @param *pHandle - Pointer on the handle used for I2C communication. Part of a struct that defines the communication module consisting of the handle, 
//
// @param ui8DevAddr - Device address
//
// @param ui32InstrLen - length of instruction
//
// @param ui64Instr - instruction/Command
//
// @param bCont hold the Bus for more instructions?
//
// @param *pData - pointer on the databuffer
//
// @param ui32NumBytes - number of bytes to write
//*****************************************************************************
uint32_t am_device_command_write(void *pHandle, uint8_t ui8DevAddr, uint32_t ui32InstrLen,
                        uint64_t ui64Instr, bool bCont,
                        uint32_t *pData, uint32_t ui32NumBytes);
						

						
						
						
						
#endif