//*****************************************************************************
//
//! @file am_devices_mb85rc64ta.c
//!
//! @brief Fujitsu 64K I2C FRAM driver.
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2021, Ambiq Micro, Inc.
// All rights reserved.
//
// edited by Timm Luhmann (IMTEK)
//
// adapted for 64k device
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_sdk_3_0_0-742e5ac27c of the AmbiqSuite Development Package.
//
//*****************************************************************************


#include "mb85rc64ta.h"
#include "gpio.h"
#include "pins.h"
#include "i2c.h"

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************

typedef struct
{
    uint32_t                    ui32Module;
    void                        *pIomHandle;
    bool                        bOccupied;
} am_devices_iom_mb85rc64ta_t;

am_devices_iom_mb85rc64ta_t gAmMb85rc64ta[AM_DEVICES_MB85RC64TA_MAX_DEVICE_NUM];	//one device maximum

am_hal_iom_config_t     g_sIomMb85rc64taCfg =
{
    .eInterfaceMode       = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq        = AM_HAL_IOM_100KHZ,
    .eSpiMode             = AM_HAL_IOM_SPI_MODE_0,
    .ui32NBTxnBufLength   = 0,
    .pNBTxnBuf = NULL,
};






//*****************************************************************************
//
//! @brief Initialize the mb85rc64ta driver.
//!
//! @param psIOMSettings  - IOM device structure describing the target spiflash.
//! @param pDMACtrlBuffer - DMA Transfer Control Buffer.
//!
//! This function should be called before any other am_devices_mb85rc64ta
//! functions. It is used to set tell the other functions how to communicate
//! with the external spiflash hardware.
//!
//! @return Status.
//
// Replaced by device_I2C_init(...)
//*****************************************************************************
//uint32_t
//am_devices_mb85rc64ta_init(uint32_t ui32Module, am_devices_mb85rc64ta_config_t *pDevConfig, void **ppHandle, void **ppIomHandle)
//{
//    void *pIomHandle;
//    am_hal_iom_config_t     stIOMMB85RC64TASettings;
//    uint32_t      ui32Index = 0;

//    // Allocate a vacant device handle
//    for ( ui32Index = 0; ui32Index < AM_DEVICES_MB85RC64TA_MAX_DEVICE_NUM; ui32Index++ )
//    {
//        if ( gAmMb85rc64ta[ui32Index].bOccupied == false )
//        {
//            break;
//        }
//    }
//    if ( ui32Index == AM_DEVICES_MB85RC64TA_MAX_DEVICE_NUM )
//    {
//        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
//    }

//    if ( (ui32Module > AM_REG_IOM_NUM_MODULES) || (pDevConfig == NULL) )
//    {
//        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
//    }

//    //
//    // Enable fault detection.
//    //
//#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
//    am_hal_fault_capture_enable();
//#endif
//#if defined (AM_PART_APOLLO3) || defined (AM_PART_APOLLO3P)
//#if AM_APOLLO3_MCUCTRL
//    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_ENABLE, 0);
//#else // AM_APOLLO3_MCUCTRL
//    am_hal_mcuctrl_fault_capture_enable();
//#endif // AM_APOLLO3_MCUCTRL
//#endif

//    stIOMMB85RC64TASettings = g_sIomMb85rc64taCfg;
//    stIOMMB85RC64TASettings.ui32NBTxnBufLength = pDevConfig->ui32NBTxnBufLength;
//    stIOMMB85RC64TASettings.pNBTxnBuf = pDevConfig->pNBTxnBuf;
//    stIOMMB85RC64TASettings.ui32ClockFreq = pDevConfig->ui32ClockFreq;

//    //
//    // Initialize the IOM instance.
//    // Enable power to the IOM instance.
//    // Configure the IOM for Serial operation during initialization.
//    // Enable the IOM.
//    //
//    if (am_hal_iom_initialize(ui32Module, &pIomHandle) ||
//        am_hal_iom_power_ctrl(pIomHandle, AM_HAL_SYSCTRL_WAKE, false) ||
//        am_hal_iom_configure(pIomHandle, &stIOMMB85RC64TASettings) ||
//        am_hal_iom_enable(pIomHandle))
//    {
//        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
//    }
//    else
//    {
//        //
//        // Configure the IOM pins.
//        //
//        //am_bsp_iom_pins_enable(ui32Module, AM_HAL_IOM_I2C_MODE);
//		am_hal_gpio_pinconfig(I2CSCL,  g_AM_BSP_GPIO_IOM1_SCL);
//		am_hal_gpio_pinconfig(I2CSDA,  g_AM_BSP_GPIO_IOM1_SDA);

//        gAmMb85rc64ta[ui32Index].bOccupied = true;
//        gAmMb85rc64ta[ui32Index].ui32Module = ui32Module;
//        *ppIomHandle = gAmMb85rc64ta[ui32Index].pIomHandle = pIomHandle;
//        *ppHandle = (void *)&gAmMb85rc64ta[ui32Index];
//        //
//        // Return the status.
//        //
//        return AM_DEVICES_MB85RC64TA_STATUS_SUCCESS;
//    }
//}

//*****************************************************************************
//
//! @brief De-Initialize the mb85rc64ta driver.
//!
//! @param ui32Module     - IOM Module#
//!
//! This function reverses the initialization
//!
//! @return Status.
//
// replaced by device_I2C_term
//*****************************************************************************
//uint32_t
//am_devices_mb85rc64ta_term(void *pHandle)
//{
//    am_devices_iom_mb85rc64ta_t *pIom = (am_devices_iom_mb85rc64ta_t *)pHandle;

//    if ( pIom->ui32Module > AM_REG_IOM_NUM_MODULES )
//    {
//        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
//    }

//    //
//    // Disable the IOM.
//    //
//    am_hal_iom_disable(pIom->pIomHandle);

//    //
//    // Disable power to and uninitialize the IOM instance.
//    //
//    am_hal_iom_power_ctrl(pIom->pIomHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);

//    am_hal_iom_uninitialize(pIom->pIomHandle);

//    // Free this device handle
//    pIom->bOccupied = false;

//    //
//    // Configure the IOM pins.
//    //
//    //am_bsp_iom_pins_disable(pIom->ui32Module, AM_HAL_IOM_I2C_MODE);
//	am_hal_gpio_pinconfig(I2CSCL,  g_AM_HAL_GPIO_DISABLE);
//	am_hal_gpio_pinconfig(I2CSDA,  g_AM_HAL_GPIO_DISABLE);

//    //
//    // Return the status.
//    //
//    return AM_DEVICES_MB85RC64TA_STATUS_SUCCESS;
//}


//*****************************************************************************
//! @error does not work
//! @brief Reads the ID of the external flash and returns the value.
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//!
//! This function reads the device ID register of the external flash, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mb85rc64ta_read_id(uint32_t *pDeviceID)
{
    //am_devices_iom_mb85rc64ta_t *pIom = (am_devices_iom_mb85rc64ta_t *)pHandle;
    am_devices_iom_mb85rc64ta_t *pIom = (am_devices_iom_mb85rc64ta_t *)my_IomdevHdl;
    //
    // Send the command sequence to read the Device ID.
    //
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_MB85RC64TA_SLAVE_ID, 1,
                           0xF9,
                           false, pDeviceID, 3))
    {
        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RC64TA_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param ui32DeviceNumber - Device number of the external flash
//! @param pui8TxBuffer - Buffer to write the external flash data from
//! @param ui32WriteAddress - Address to write to in the external flash
//! @param ui32NumBytes - Number of bytes to write to the external flash
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external flash at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target flash
//! memory or underflow the pui8TxBuffer array
//
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mb85rc64ta_blocking_write(uint8_t *pui8TxBuffer,
                                     uint32_t ui32WriteAddress,
                                     uint32_t ui32NumBytes)
{
    //am_devices_iom_mb85rc64ta_t *pIom = (am_devices_iom_mb85rc64ta_t *)pHandle;
	am_devices_iom_mb85rc64ta_t *pIom = (am_devices_iom_mb85rc64ta_t *)my_IomdevHdl;
    //
    // Write the data to the device.
    //
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_MB85RC64TA_SLAVE_ID, 2,
                            (ui32WriteAddress & 0x0000FFFF),
                            false, (uint32_t *)pui8TxBuffer, ui32NumBytes))
    {
        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RC64TA_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param ui32DeviceNumber - Device number of the external flash
//! @param pui8TxBuffer - Buffer to write the external flash data from
//! @param ui32WriteAddress - Address to write to in the external flash
//! @param ui32NumBytes - Number of bytes to write to the external flash
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external flash at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target flash
//! memory or underflow the pui8TxBuffer array
//
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mb85rc64ta_nonblocking_write(uint8_t *pui8TxBuffer,
                                        uint32_t ui32WriteAddress,
                                        uint32_t ui32NumBytes,
                                        am_hal_iom_callback_t pfnCallback,
                                        void *pCallbackCtxt)
{
    am_hal_iom_transfer_t         Transaction;
    //am_devices_iom_mb85rc64ta_t   *pIom = (am_devices_iom_mb85rc64ta_t *)pHandle;
	am_devices_iom_mb85rc64ta_t *pIom = (am_devices_iom_mb85rc64ta_t *)my_IomdevHdl;
	
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.uPeerInfo.ui32I2CDevAddr = AM_DEVICES_MB85RC64TA_SLAVE_ID;
    Transaction.bContinue       = false;

    //
    // Set up the IOM transaction.
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 2;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P))
    Transaction.ui64Instr       = ui32WriteAddress & 0x0000FFFF;
#else
    Transaction.ui32Instr       = ui32WriteAddress & 0x0000FFFF;
#endif
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer;

    //
    // Add this transaction to the command queue (no callback).
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, pfnCallback, pCallbackCtxt))
    {
        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RC64TA_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pui8RxBuffer - Buffer to store the received data from the flash
//! @param ui32ReadAddress - Address of desired data in external flash
//! @param ui32NumBytes - Number of bytes to read from external flash
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mb85rc64ta_blocking_read(uint8_t *pui8RxBuffer,
                                    uint32_t ui32ReadAddress,
                                    uint32_t ui32NumBytes)
{
    //am_devices_iom_mb85rc64ta_t   *pIom = (am_devices_iom_mb85rc64ta_t *)pHandle;
	am_devices_iom_mb85rc64ta_t *pIom = (am_devices_iom_mb85rc64ta_t *)my_IomdevHdl;

    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_MB85RC64TA_SLAVE_ID, 2,
                           (ui32ReadAddress & 0x0000FFFF),
                           false, (uint32_t *)pui8RxBuffer, ui32NumBytes))
    {
        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RC64TA_STATUS_SUCCESS;
}


//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pui8RxBuffer - Buffer to store the received data from the flash
//! @param ui32ReadAddress - Address of desired data in external flash
//! @param ui32NumBytes - Number of bytes to read from external flash
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mb85rc64ta_nonblocking_read(uint8_t *pui8RxBuffer,
                                       uint32_t ui32ReadAddress,
                                       uint32_t ui32NumBytes,
                                       am_hal_iom_callback_t pfnCallback,
                                       void * pCallbackCtxt)
{
    am_hal_iom_transfer_t Transaction;
    //am_devices_iom_mb85rc64ta_t   *pIom = (am_devices_iom_mb85rc64ta_t *)pHandle;
	am_devices_iom_mb85rc64ta_t *pIom = (am_devices_iom_mb85rc64ta_t *)my_IomdevHdl;

    //
    // Set up the IOM transaction.
    //
    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32InstrLen    = 2;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P))
    Transaction.ui64Instr       = (ui32ReadAddress & 0x0000FFFF);
#else
    Transaction.ui32Instr       = (ui32ReadAddress & 0x0000FFFF);
#endif
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32RxBuffer   = (uint32_t *)pui8RxBuffer;
    Transaction.uPeerInfo.ui32I2CDevAddr = AM_DEVICES_MB85RC64TA_SLAVE_ID;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    Transaction.bContinue       = false;

    //
    // Start the transaction.
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, pfnCallback, pCallbackCtxt))
    {
        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RC64TA_STATUS_SUCCESS;
}

