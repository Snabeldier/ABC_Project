/*!
 *  \file       spi.c
 *
 *  \brief      Provides functions to use the spi driver
 *
 *  \date       13.04.2023
 *
 *  \author     Timm Luhmann (IMTEK)
 */
//***** Header Files **********************************************************
//#include <stdint.h>
//#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "pins.h"
#include "spi.h"
#include "gpio.h"
#include "pins.h"
#include "boardControl.h"

void *my_pIomSPIHandle;

void *my_IomSPIdevHdl;

uint32_t SPIbuffer[256];

typedef struct
{
    uint32_t                    ui32Module;
	uint32_t					ui32CS;
    void                        *pIomHandle;
    bool                        bOccupied;
} device_SPI;

device_SPI SPI_dev[DEVICE_SPI_MAX_DEVICE_NUM];	//one device maximum

am_hal_iom_config_t     SPI_cfg =
{
    .eInterfaceMode       = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq        = AM_HAL_IOM_1MHZ,
    .eSpiMode             = AM_HAL_IOM_SPI_MODE_0,
    .ui32NBTxnBufLength   = 0,
    .pNBTxnBuf = NULL,
};


//*****************************************************************************
//
//! @brief Initialize the mb85rs1mt driver.
//!
//! @param ui32Module     - IOM Module#
//! @param psIOMSettings  - IOM device structure describing the target spiflash.
//!
//! This function should be called before any other am_devices_mb85rs1mt
//! functions. It is used to set tell the other functions how to communicate
//! with the external spiflash hardware.
//!
//! @return Status.
//
//*****************************************************************************
uint32_t
spi_init()
{
    void *pIomHandle;
    am_hal_iom_config_t     stIOMMSPISettings;
	uint32_t ui32Module = 0;		//always IOM0 for SPI
	spi_config_t spiTester;
	spiTester.ui32ClockFreq = AM_HAL_IOM_1MHZ;
	spiTester.pNBTxnBuf = SPIbuffer;
	spiTester.ui32NBTxnBufLength = sizeof(SPIbuffer);
	spi_config_t *pDevConfig = &spiTester;
    uint32_t g_CS[AM_REG_IOM_NUM_MODULES] = {2};;	//CS 2 is used by the corresponding pin


    uint32_t      ui32Index = 0;

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < DEVICE_SPI_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( SPI_dev[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == DEVICE_SPI_MAX_DEVICE_NUM )
    {
        return SPI_STATUS_ERROR;
    }

    if ( (ui32Module > AM_REG_IOM_NUM_MODULES)  || (pDevConfig == NULL) )
    {
        return SPI_STATUS_ERROR;
    }

    //
    // Configure the IOM pins.
    //
    //am_bsp_iom_pins_enable(ui32Module, AM_HAL_IOM_SPI_MODE);
	am_hal_gpio_pinconfig(SPIMASTER0CLK, g_AM_BSP_GPIO_DSPL1_SCL);
	am_hal_gpio_pinconfig(SPIMASTER0MISO, g_AM_BSP_GPIO_DSPL1_SDO);
	am_hal_gpio_pinconfig(SPIMASTER0MOSI,g_AM_BSP_GPIO_DSPL1_SDI);
	am_hal_gpio_pinconfig(CS_LORA, g_AM_BSP_GPIO_DSPL1_CS);
    //
    // Enable fault detection.
    //
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    am_hal_fault_capture_enable();
#else
#if AM_APOLLO3_MCUCTRL
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_ENABLE, 0);
#else // AM_APOLLO3_MCUCTRL
    am_hal_mcuctrl_fault_capture_enable();
#endif // AM_APOLLO3_MCUCTRL
#endif

    stIOMMSPISettings = SPI_cfg;
    stIOMMSPISettings.ui32NBTxnBufLength = pDevConfig->ui32NBTxnBufLength;
    stIOMMSPISettings.pNBTxnBuf = pDevConfig->pNBTxnBuf;
    stIOMMSPISettings.ui32ClockFreq = pDevConfig->ui32ClockFreq;

    //
    // Initialize the IOM instance.
    // Enable power to the IOM instance.
    // Configure the IOM for Serial operation during initialization.
    // Enable the IOM.
    // HAL Success return is 0
    //
    if (am_hal_iom_initialize(ui32Module, &pIomHandle) ||
        am_hal_iom_power_ctrl(pIomHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_iom_configure(pIomHandle, &stIOMMSPISettings) ||
        am_hal_iom_enable(pIomHandle))
    {
        return SPI_STATUS_ERROR;
    }
    else
    {
        SPI_dev[ui32Index].bOccupied = true;
        SPI_dev[ui32Index].ui32Module = ui32Module;
        SPI_dev[ui32Index].ui32CS = g_CS[ui32Module];
		my_pIomSPIHandle = SPI_dev[ui32Index].pIomHandle = pIomHandle;
		my_IomSPIdevHdl = (void *)&SPI_dev[ui32Index];
        //*ppIomHandle = SPI_dev[ui32Index].pIomHandle = pIomHandle;
        //*ppHandle = (void *)&SPI_dev[ui32Index];
        //
        // Return the status.
        //
        return SPI_STATUS_SUCCESS;
    }
}


//*****************************************************************************
//
//! @brief De-Initialize the mb85rs1mt driver.
//!
//! @param ui32Module     - IOM Module#
//!
//! This function reverses the initialization
//!
//! @return Status.
//
//*****************************************************************************
uint32_t
spi_term()
{
    device_SPI *pIom = (device_SPI *)my_pIomSPIHandle;

    if ( pIom->ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return SPI_STATUS_ERROR;
    }

    // Disable the pins
    //am_bsp_iom_pins_disable(pIom->ui32Module, AM_HAL_IOM_SPI_MODE);
	am_hal_gpio_pinconfig(SPIMASTER0CLK, g_AM_HAL_GPIO_DISABLE);
	am_hal_gpio_pinconfig(SPIMASTER0MISO, g_AM_HAL_GPIO_DISABLE);
	am_hal_gpio_pinconfig(SPIMASTER0MOSI,g_AM_HAL_GPIO_DISABLE);
	am_hal_gpio_pinconfig(CS_LORA, g_AM_HAL_GPIO_DISABLE);
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
    // Return the status.
    //
    return SPI_STATUS_SUCCESS;
}


//*****************************************************************************
//
// Generic Command Write function.
//
//*****************************************************************************
uint32_t
spi_command_write(uint32_t ui32InstrLen, uint64_t ui64Instr,
                        uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    device_SPI *pIom = (device_SPI *)my_IomSPIdevHdl;

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
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
		am_util_delay_ms(1);
        return SPI_STATUS_ERROR;
    }
	am_util_delay_ms(1);
    return SPI_STATUS_SUCCESS;
}

uint32_t
spi_command_read(uint32_t ui32InstrLen, uint64_t ui64Instr,
                       uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    device_SPI *pIom = (device_SPI *)my_IomSPIdevHdl;

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
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
		am_util_delay_ms(1);
        return SPI_STATUS_ERROR;
    }
	am_util_delay_ms(1);
    return SPI_STATUS_SUCCESS;
}