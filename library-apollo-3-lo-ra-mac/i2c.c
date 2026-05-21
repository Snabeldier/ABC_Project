/*!
 *  \file       i2c.c
 *
 *  \brief      Provides functions to use the i2c driver
 *
 *  \date       02.02.2023
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
//***** Header Files **********************************************************
//#include <stdint.h>
//#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "pins.h"
#include "i2c.h"
#include "mb85rc64ta.h"
#include "gpio.h"
#include "pins.h"
#include "device.h"


//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************



void *my_pIomHandle;

void *my_IomdevHdl;

uint32_t I2Cbuffer[256];

typedef struct
{
    uint32_t                    ui32Module;
    void                        *pIomHandle;
    bool                        bOccupied;
} device_I2C;

device_I2C I2C_dev[DEVICE_I2C_MAX_DEVICE_NUM];	//one device maximum

am_hal_iom_config_t     I2C_cfg =
{
    .eInterfaceMode       = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq        = AM_HAL_IOM_400KHZ,
    .eSpiMode             = AM_HAL_IOM_SPI_MODE_0,
    .ui32NBTxnBufLength   = 0,
    .pNBTxnBuf = NULL,
};



//*****************************************************************************
//
//! @brief Initialize the I2C driver.
//!
//! @param ui32Module  - IOM module - for I2C always 1
//! @param pDMACtrlBuffer - DMA Transfer Control Buffer.
//! @deprecated Settings are now done inside the function to reduce complexity for now
//! This function should be called before any other I2C
//! functions. 
//!
//! @return Status.
//
//*****************************************************************************
uint32_t
//device_I2C_init(uint32_t ui32Module, device_I2C_config_t *pDevConfig, void **ppHandle, void **ppIomHandle)
//device_I2C_init(uint32_t ui32Module, device_I2C_config_t *pDevConfig)
device_I2C_init(void)
{
	turnOnI2C();
    device_I2C_config_t i2cTester;
    i2cTester.ui32ClockFreq = AM_HAL_IOM_400KHZ;
    i2cTester.ui32NBTxnBufLength = sizeof(I2Cbuffer);
    i2cTester.pNBTxnBuf = I2Cbuffer;
    uint32_t ui32Module = 1;        // always IOM1 for I2C
    device_I2C_config_t *pDevConfig = &i2cTester;
    void *pIomHandle;
    am_hal_iom_config_t     stIOMI2CSettings;
    uint32_t      ui32Index = 0;

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < DEVICE_I2C_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( I2C_dev[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == DEVICE_I2C_MAX_DEVICE_NUM )
    {
        return DEVICE_I2C_STATUS_ERROR;
    }

    if ( (ui32Module > AM_REG_IOM_NUM_MODULES) || (pDevConfig == NULL) )
    {
        return DEVICE_I2C_STATUS_ERROR;
    }

    //
    // Enable fault detection.
    //
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    am_hal_fault_capture_enable();
#endif
#if defined (AM_PART_APOLLO3) || defined (AM_PART_APOLLO3P)
#if AM_APOLLO3_MCUCTRL
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_ENABLE, 0);
#else // AM_APOLLO3_MCUCTRL
    am_hal_mcuctrl_fault_capture_enable();
#endif // AM_APOLLO3_MCUCTRL
#endif

    stIOMI2CSettings = I2C_cfg;
    stIOMI2CSettings.ui32NBTxnBufLength = pDevConfig->ui32NBTxnBufLength;
    stIOMI2CSettings.pNBTxnBuf = pDevConfig->pNBTxnBuf;
    stIOMI2CSettings.ui32ClockFreq = pDevConfig->ui32ClockFreq;

    //
    // Initialize the IOM instance.
    // Enable power to the IOM instance.
    // Configure the IOM for Serial operation during initialization.
    // Enable the IOM.
    //
    if (am_hal_iom_initialize(ui32Module, &pIomHandle) ||
        am_hal_iom_power_ctrl(pIomHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_iom_configure(pIomHandle, &stIOMI2CSettings) ||
        am_hal_iom_enable(pIomHandle))
    {
        return DEVICE_I2C_STATUS_ERROR;
    }
    else
    {
        //
        // Configure the IOM pins.
        //
        //am_bsp_iom_pins_enable(ui32Module, AM_HAL_IOM_I2C_MODE);
		am_hal_gpio_pinconfig(I2CSCL,  g_AM_BSP_GPIO_IOM1_SCL);
		am_hal_gpio_pinconfig(I2CSDA,  g_AM_BSP_GPIO_IOM1_SDA);

        I2C_dev[ui32Index].bOccupied = true;
        I2C_dev[ui32Index].ui32Module = ui32Module;
        my_pIomHandle = I2C_dev[ui32Index].pIomHandle = pIomHandle;
        my_IomdevHdl = (void *)&I2C_dev[ui32Index];
//		*ppIomHandle = I2C_dev[ui32Index].pIomHandle = pIomHandle;
//        *ppHandle = (void *)&I2C_dev[ui32Index];
		//my_pIomHandle = *ppIomHandle;
		//my_IomdevHdl = *ppHandle;
        //
        // Return the status.
        //
        return DEVICE_I2C_STATUS_SUCCESS;
    }
}

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
device_I2C_term(void)
{
    device_I2C *pIom = (device_I2C *)my_pIomHandle;
	
    if ( pIom->ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return DEVICE_I2C_STATUS_ERROR;
    }

    //
    // Disable the IOM.
    //
    am_hal_iom_disable(pIom->pIomHandle);

    //
    // Disable power to and uninitialize the IOM instance.
    //
    am_hal_iom_power_ctrl(pIom->pIomHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);

    am_hal_iom_uninitialize(pIom->pIomHandle);

    // Free this device handle
    pIom->bOccupied = false;

    //
    // Configure the IOM pins.
    //
    //am_bsp_iom_pins_disable(pIom->ui32Module, AM_HAL_IOM_I2C_MODE);
	am_hal_gpio_pinconfig(I2CSCL,  g_AM_HAL_GPIO_DISABLE);
	am_hal_gpio_pinconfig(I2CSDA,  g_AM_HAL_GPIO_DISABLE);

    //
    // Return the status.
    //
    return DEVICE_I2C_STATUS_SUCCESS;
}



//*****************************************************************************
//
// Generic Command Write function.
//
//*****************************************************************************
uint32_t am_device_command_write(void *pHandle, uint8_t ui8DevAddr, uint32_t ui32InstrLen,
                        uint64_t ui64Instr, bool bCont,
                        uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = ui32InstrLen;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P))
    Transaction.ui64Instr       = ui64Instr;
#else
    Transaction.ui32Instr       = (uint32_t)ui64Instr;
#endif
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = pData;
    Transaction.uPeerInfo.ui32I2CDevAddr = ui8DevAddr;
    Transaction.bContinue       = bCont;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pHandle, &Transaction))
    {
        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
    }
    return AM_DEVICES_MB85RC64TA_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Generic Command Read function.
//
//*****************************************************************************
uint32_t am_device_command_read(void *pHandle, uint8_t ui8DevAddr, uint32_t ui32InstrLen, uint64_t ui64Instr,
                       bool bCont, uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t  Transaction;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = ui32InstrLen;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P))
    Transaction.ui64Instr       = ui64Instr;
#else
    Transaction.ui32Instr       = (uint32_t)ui64Instr;
#endif
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32RxBuffer   = pData;
    Transaction.uPeerInfo.ui32I2CDevAddr = ui8DevAddr;
    Transaction.bContinue       = bCont;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pHandle, &Transaction))
    {
        return AM_DEVICES_MB85RC64TA_STATUS_ERROR;
    }
    return AM_DEVICES_MB85RC64TA_STATUS_SUCCESS;
}




