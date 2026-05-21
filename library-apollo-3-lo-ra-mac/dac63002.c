/*!
 *  \file       dac63002.h
 *
 *  \brief      Provides functions to use the external DAC
 *
 *  \date       15.02.2023
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
 
 
 #include "dac63002.h"
 
 
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
} am_devices_iom_dac63002_t;

am_devices_iom_dac63002_t gDac63002[AM_DEVICES_DAC63002_MAX_DEVICE_NUM];	//one device maximum

am_hal_iom_config_t     g_sIomDac63002Cfg =
{
    .eInterfaceMode       = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq        = AM_HAL_IOM_100KHZ,
    .eSpiMode             = AM_HAL_IOM_SPI_MODE_0,
    .ui32NBTxnBufLength   = 0,
    .pNBTxnBuf = NULL,
};

//*****************************************************************************
//
//! @brief Reads the ID of the DAC and returns the value.
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//!
//! This function reads the device ID register of the external rtc, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_read_id(uint32_t *pDeviceID)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;

    //
    // Send the command sequence to read the Device ID.
    //
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           0x22,
                           false, pDeviceID, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	*pDeviceID = dataPrepare(*pDeviceID);
	if(*pDeviceID & DEVICE_ID<<2){
		return AM_DEVICES_DAC63002_STATUS_SUCCESS;
	}
    //
    // Return the status.
    //
    return AM_DEVICES_DAC63002_STATUS_ERROR;
}

//*****************************************************************************
//
//! @brief Sets the gain for the DAC voltage output mode
//!
//! @param gain 0: Gain = 1x external VREF,
//!		   gain 1: Gain = 1x VDD
//!		   gain 2: Gain = 1.5x internal Reference (1.21V)
//!		   gain 3: Gain = 2x internal reference
//!		   gain 4: Gain = 3x internal reference
//!		   gain 5: Gain = 4x internal reference
//! This function sets the gain for the DAC in voltage output mode
//! It keeps the other setting in this register
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_gain(uint8_t gain)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = 0;
	//
    // Send the command sequence to read the DAC1 Vout CMP config register
    //
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_VOUT_CMP_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	dataByte = dataPrepare(dataByte);
    //
    // Write the DAC1 Vout CMP config register
    //
	if(gain & 0x01)
	{
		dataByte |= 1<<10;
	}
	else
	{
		dataByte &= ~(1<<10);
	}
	if(gain & 0x02)
	{
		dataByte |= 1<<11;
	}
	else
	{
		dataByte &= ~(1<<11);
	}
	if(gain & 0x04)
	{
		dataByte |= 1<<12;
	}
	else
	{
		dataByte &= ~(1<<12);
	}
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_VOUT_CMP_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}



//*****************************************************************************
//
//! @brief Disable the comparator functionality on channel 1
//!

//! This function disables the comparator functionality on channel 1 &
//! sets the output pin in push-pull mode for the DAC operation
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_disable_CMP()
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = 0;
	//
    // Send the command sequence to read the DAC1 Vout CMP config register
    //
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_VOUT_CMP_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	dataByte = dataPrepare(dataByte);
	
    //
    // Write the DAC1 Vout CMP config register
    //
	dataByte &= ~(0x1F);	//clear bit 0-4
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_VOUT_CMP_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}


//*****************************************************************************
//
//! @brief Sets the output range of the iOut mode
//!
//! @param iOut 0: 0 to 25�A
//!		   iOut 1: 0 to 50�A
//!		   iOut 2: 0 to 125�A
//!		   iOut 3: 0 to 250�A
//!		   iOut 4: 0 to -24�A
//!		   iOut 5: 0 to -48�A
//!		   iOut 6: 0 to -120�A
//!		   iOut 7: 0 to -240�A
//!		   iOut 8: -25�A to 25�A
//!		   iOut 9: -50�A to 50�A
//!		   iOut 10: -125�A to 125�A
//!		   iOut 11: -250�A to 250�A
//! This function sets the output range in i out mode
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_Iout_Range(uint8_t iOut)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = 0;
    //
    // Write the DAC1 Vout CMP config register
    //
	if(iOut & 0x01)
	{
		dataByte |= 1<<9;
	}
	else
	{
		dataByte &= ~(1<<9);
	}
	if(iOut & 0x02)
	{
		dataByte |= 1<<10;
	}
	else
	{
		dataByte &= ~(1<<10);
	}
	if(iOut & 0x04)
	{
		dataByte |= 1<<11;
	}
	else
	{
		dataByte &= ~(1<<11);
	}
	if(iOut & 0x08)
	{
		dataByte |= 1<<12;
	}
	else
	{
		dataByte &= ~(1<<12);
	}
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_IOUT_MISC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}


//*****************************************************************************
//
//! @brief Set the DAC1 CMP hysteresis or window function
//!
//! @param hysteresis 0: no hysteresis
//!					  1: hysteresis using DAC1 Margin High&Low
//!					  2: window comparator mode with DAC1 Margin High&Low setting the window bound
//! This function sets the DAC1 CMP mode either in hysteresis, window or none
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_CMP_CONFIG(uint8_t hysteresis)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = 0;

	
    //
    // Write the DAC1 CMP mode config register
    //
	dataByte |= (hysteresis<<10);
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_VOUT_CMP_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}



//*****************************************************************************
//
//! @brief Clear the DAC1 to Zero/Mid-scale
//!
//! @param clearSel: 0->set to zero scale
//!					 1->set to mid scale
//! This function disables the comparator functionality on channel 1 &
//! sets the output pin in push-pull mode for the DAC operation
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_clear(uint8_t clearSel)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = 0;
	//
    // Send the command sequence to read the DAC1 Vout CMP config register
    //
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_FUNC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	dataByte = dataPrepare(dataByte);
    //
    // Write the DAC1 Vout CMP config register
    //
	if(clearSel)
	{
		dataByte |= (1<<15);
	}
	else
	{
		dataByte &= ~(1<<15);
	}
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_FUNC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}




//*****************************************************************************
//
//! @brief Set the DAC1 broadcast setting
//!
//! @param broadcast: 0->Dont update DAC1 with broadcast command
//!					  1->Update DAC1 with broadcast command
//! This function disables the comparator functionality on channel 1 &
//! sets the output pin in push-pull mode for the DAC operation
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_broadcast(uint8_t broadcast)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = 0;
	//
    // Send the command sequence to read the DAC1 Vout CMP config register
    //
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_FUNC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	dataByte = dataPrepare(dataByte);
    //
    // Write the DAC1 Vout CMP config register
    //
	if(broadcast)
	{
		dataByte |= (1<<13);
	}
	else
	{
		dataByte &= ~(1<<13);
	}
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_FUNC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}



//*****************************************************************************
//
//! @brief Set the DAC1 broadcast setting
//!
//! @param phase 0: 0�
//!				 1: 120�
//!				 2: 240�
//!				 3: 90�
//! @param waveform 0: Triangular Wave
//!					1: Sawtooth Wave
//!					2: Inverse Sawtooth Wave
//!					4: Sine Wave
//!					7: Disable
//! This function disables the comparator functionality on channel 1 &
//! sets the output pin in push-pull mode for the DAC operation
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_function_generation(uint8_t phase, uint8_t waveform)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = 0;
	//
    // Send the command sequence to read the DAC1 Vout CMP config register
    //
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_FUNC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	dataByte = dataPrepare(dataByte);
    //
    // determine & set phase setting
    //
	if(phase & 0x01)
	{
		dataByte |= (1<<11);
	}
	else
	{
		dataByte &= ~(1<<11);
	}
	if(phase & 0x02)
	{
		dataByte |= (1<<12);
	}
	else
	{
		dataByte &= ~(1<<12);
	}
	// determine & set function setting
	if(waveform & 0x01)
	{
		dataByte |= (1<<8);
	}
	else
	{
		dataByte &= ~(1<<8);
	}
	if(waveform & 0x02)
	{
		dataByte |= (1<<9);
	}
	else
	{
		dataByte &= ~(1<<9);
	}
	if(waveform & 0x04)
	{
		dataByte |= (1<<10);
	}
	else
	{
		dataByte &= ~(1<<10);
	}
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_FUNC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}


//*****************************************************************************
//
//! @brief Set the DAC1 linear slew rate
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//! @param codeStep 0: 1-LSB
//!				    1: 2-LSB
//!				    2: 3-LSB
//!				    3: 4-LSB
//!				    4: 6-LSB
//!				    5: 8-LSB
//!				    6: 16-LSB
//!				    7: 32-LSB
//! @param slewRate 0: no slewrate, invalid for waveform generation
//!					1: 4�s/step
//!					2: 8�s/step
//!					3: 12�s/step
//!					4: 18�s/step
//!					5: 27.04�s/step
//!					6: 40.48�s/step
//!					7: 60.72�s/step
//!					8: 91.12�s/step
//!					9: 136.72�s/step
//!					10: 239.2�s/step
//!					11: 418.64�s/step
//!					12: 732.56�s/step
//!					13: 1282�s/step
//!					14: 2563.96�s/step
//!					15: 5127.92�s/step
//! This function sets the slew rate for linear mode
//! sets the output pin in push-pull mode for the DAC operation
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_linear_slew(uint8_t codeStep, uint8_t slewRate)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = 0;
	//
    // Send the command sequence to read the DAC1 Vout CMP config register
    //
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_FUNC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	dataByte = dataPrepare(dataByte);
	dataByte &= ~(1<<7);	//set to linear slew
	if(codeStep & 0x01)
	{
		dataByte |= (1<<4);
	}
	else
	{
		dataByte &= ~(1<<4);
	}
	if(codeStep & 0x02)
	{
		dataByte |= (1<<5);
	}
	else
	{
		dataByte &= ~(1<<5);
	}
	if(codeStep & 0x04)
	{
		dataByte |= (1<<6);
	}
	else
	{
		dataByte &= ~(1<<6);
	}
	// determine & set function setting
	if(slewRate & 0x01)
	{
		dataByte |= (1);
	}
	else
	{
		dataByte &= ~(1);
	}
	if(slewRate & 0x02)
	{
		dataByte |= (1<<1);
	}
	else
	{
		dataByte &= ~(1<<1);
	}
	if(slewRate & 0x04)
	{
		dataByte |= (1<<2);
	}
	else
	{
		dataByte &= ~(1<<2);
	}
	if(slewRate & 0x08)
	{
		dataByte |= (1<<3);
	}
	else
	{
		dataByte &= ~(1<<3);
	}
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_FUNC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}



//*****************************************************************************
//
//! @brief Set the DAC1 logarithmic slew rate
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//! @param riseSlew 0: 4�S/step
//!				    1: 12�s/step
//!				    2: 27.04�s/step
//!				    3: 60.72�s/step
//!				    4: 136.72�s/step
//!				    5: 418.64�s/step
//!				    6: 1282�s/step
//!				    7: 5127.92�s/step
//! @param fallSlew 0: 4�S/step
//!				    1: 12�s/step
//!				    2: 27.04�s/step
//!				    3: 60.72�s/step
//!				    4: 136.72�s/step
//!				    5: 418.64�s/step
//!				    6: 1282�s/step
//!				    7: 5127.92�s/step
//! This function disables the comparator functionality on channel 1 &
//! sets the output pin in push-pull mode for the DAC operation
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_logarithmic_slew(uint8_t riseSlew, uint8_t fallSlew)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = 0;
	//
    // Send the command sequence to read the DAC1 Vout CMP config register
    //
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_FUNC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	dataByte = dataPrepare(dataByte);
	dataByte |= (1<<7);
	if(riseSlew & 0x01)
	{
		dataByte |= (1<<4);
	}
	else
	{
		dataByte &= ~(1<<4);
	}
	if(riseSlew & 0x02)
	{
		dataByte |= (1<<5);
	}
	else
	{
		dataByte &= ~(1<<5);
	}
	if(riseSlew & 0x04)
	{
		dataByte |= (1<<6);
	}
	else
	{
		dataByte &= ~(1<<6);
	}
	// determine & set function setting
	if(fallSlew & 0x01)
	{
		dataByte |= (1<<1);
	}
	else
	{
		dataByte &= ~(1<<1);
	}
	if(fallSlew & 0x02)
	{
		dataByte |= (1<<2);
	}
	else
	{
		dataByte &= ~(1<<2);
	}
	if(fallSlew & 0x04)
	{
		dataByte |= (1<<3);
	}
	else
	{
		dataByte &= ~(1<<3);
	}
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_FUNC_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}


//*****************************************************************************
//
//! @brief Set the DAC1 data register aka the output value
//!
//! @param value: 12-bit value for the output
//!
//! This function sets the output value for the DAC
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_output(uint16_t value)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = value;

	//dataByte |= (value<<4);
	
	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_DATA,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Set the DAC1 common config register
//!
//! @param data - Buffer that contains the following:
//! @param winLatch: 0-> non-latching window comparator output, 1->latching window comparator output
//! @param devLock: 0 -> device unlocked, to unlock write first in common-trigger, 1 -> device locked
//! @param EE_read: 0 -> fault dump read enable at 0x00, 1 -> fault dump read enable at 0x01
//! @param enIntRef: 0 -> disable internal reference, 1 -> enable internal reference
//! @param pwrVout0: 0 -> power up vout0/1
//!	@param pwrVout1: 1 -> power down vout0/1 with 10k Ohm to agnd
//!				   	 2 -> power down vout0/1 with 100k Ohm to agnd
//!				   	 3 -> power with vout0/1 with Hi-Z to agnd
//! @param pwrIout0: 0 -> power up Iout0, 1 -> power down Iout0
//! @param pwrIout1: 0 -> power up Iout1, 1 -> power down Iout0
//! This function sets the common register with its corresponding values
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_common_config(uint32_t data)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = data;

	dataByte = dataPrepare(dataByte);

    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_COMMON_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}


//*****************************************************************************
//
//! @brief Set the DAC common trigger register
//!
//! @param data - Buffer that contains the data to set the common trigger. It contains the follwing:
//! @param devUnlck: 0101 -> password to unlock
//! @param reset : 1010 -> password to trigger a POR reset
//! @param LDAC: 0 -> LDAC operation not triggered, 1 -> LDAC operation triggered
//! @param CLR: 0 -> nothing happens, 1 -> resets DAC registers to what is in FUNC_CONFIG defined
//! @param faultDump: 0 -> fault dump is not triggered, 1 -> triggers fault dump sequence
//!	@param protect: 0 -> protect function is not triggered, 1 -> protect function is triggered
//!	@param readOneTring: 0 -> nothing happends, 1 -> reads one row of NVM for fault dump
//!	@param NVMProg: 0 -> NVM write not triggered, 1 -> NVM write triggered
//! @param NVMReload: 0 -> NVM reload not triggered, 1 -> reload data from NVM register map
//! This function sets the common trigger with its corresponding values
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_common_trigger(uint32_t data)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = data;
	dataByte = dataPrepare(dataByte);

    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_COMMON_TRIGGER,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}


//*****************************************************************************
//
//! @brief Set the DAC trigger register
//!
//! @param data - Buffer that contains the data which is to be written in the DAC trigger register which contains the following:
//! @param resetCMPFLAG1/0: 0 -> latching comparator uneffected, 1-> reset latching comparator & window comparator output
//! @param trigMARLO1/0 : 0 -> nothing happens, 1 -> trigger margin low command
//! @param trigMARHI1/0: 0 -> nothing happens, 1 -> trigger margin high command
//! @param startFunc1/0: 0 -> stop function generation, 1 -> start function generation
//! This function triggers the corresponding functionalities
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_trigger(uint32_t data)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = data;
	dataByte = dataPrepare(dataByte);

    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_COMMON_DAC_TRIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Read the DAC general status register
//!
//! @param data - Pointer to the return buffer which contains the following:
//! @param Bit15: 0 -> no CRC error in OTP, 1-> indicates failure in OTP loading, refer to datasheet
//! @param Bit14 : 0 -> no CRC error in NVM load, 1 -> indicates failure in NVM loading, refer to datasheet
//! @param Bit12 : 0 -> DAC0 can accept commands, 1 -> DAC0 cannot accept commands
//! @param Bit9 : 0 -> DAC1 can accept commands, 1 -> DAC1 cannot accept commands
//! @param Bit7-2: device identifier
//! @param Bit1-0: version identifier
//! This function reads the general status register of the DAC63002
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_read_general_status(uint32_t *data)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;

    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_GENERAL_STATUS,
                           false, data, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	*data = dataPrepare(*data);
    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}


//*****************************************************************************
//
//! @brief Read the DAC comparator status register
//!
//! @param data - Pointer to the return buffer for the Device ID. It contains the following:
//! @param Bit8: 0 -> PROTECT operation not triggered, 1-> protect function is completed or in progress, refer to datasheet
//! @param Bit7 : Window comparator output of channel 0
//! @param Bit4 : Window comparator output of channel 1
//! @param Bit3 : Synchronized comparator output from channel 0
//! @param Bit0 : Synchronized comparator output from channel 1
//! 
//! This function reads the cmp status register of the DAC63002
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_read_cmp_status(uint32_t *data)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;

    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_CMP_STATUS,
                           false, data, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	*data = dataPrepare(*data);
    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}



//*****************************************************************************
//
//! @brief Set device mode config register
//!
//! @param data - databyte that contains the following possible configuration
//! @param dis_mode_in : write 1 for low power mode
//! @param protected config : 0 -> switch to Hi-Z power down, 1 -> switch to DAC code stored in NVM, then switch to Hi-Z power down
//!							  2 -> slew to margin low and then to Hi-Z power down, 3 -> slew to margin high and then to Hi-Z power down
//!
//! This function sets the device mode config register
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_device_mode_config(uint32_t data)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	uint32_t dataByte = data;

	dataByte = dataPrepare(dataByte);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DEVICE_MODE_CONFIG,
                           false, &dataByte, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}
/*
* @brief Function that can be called to read the current output value of the DAC (channel 1)
*
* @param *data Pointer to the location where the data is to stored
*
* @returns Whether the operation was successful or not
*/
uint32_t
am_devices_dac63002_read_output(uint32_t *data)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;

    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           ADDRESS_DAC1_DATA,
                           false, data, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	*data = dataPrepare(*data);
    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}
/*
* @brief A function that can be called to read a register of the DAC
*
* @param *data pointer to the location where the data will be stored
* @param reg register which is target to the read operation
*
* @returns Whether the read operation was successful or not
*/
uint32_t
am_devices_dac63002_read_register(uint32_t *data, uint32_t reg)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           reg,
                           false, data, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }
	*data = dataPrepare(*data);
    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}

/*
* @brief A function that can be called to write a register of the DAC
*
* @brief data To be written data
* @brief reg register which is target to the read operation
*
* @returns Whether the write operation was succesful or not
*/
uint32_t
am_devices_dac63002_write_register(uint32_t data, uint32_t reg)
{
    am_devices_iom_dac63002_t *pIom = (am_devices_iom_dac63002_t *)my_IomdevHdl;
	
	data = dataPrepare(data);
	
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_DAC63002_SLAVE_ID, 1,
                           reg,
                           false, &data, 2))
    {
        return AM_DEVICES_DAC63002_STATUS_ERROR;
    }

    //
    // Return the status.
    //	
    return AM_DEVICES_DAC63002_STATUS_SUCCESS;
}

/*
* @brief This function is required to communicate properly with the DAC since it requieres 2 Byte packages
*        The function basically exchanges the two bytes to have them in the correct order
*
* @returns the processed data
*/
uint32_t dataPrepare(uint32_t data)
{
	uint32_t buffer = data;		//first secure data in buffer
	data = 0;						//then delete contents from variable
	data = buffer>>8;				//then put lower 8 bit
	data |= ((buffer<<8) & 0xFF00);//then put higher 8 bit
	return data;
}
/*
* @brief This function can be used to set the dac to output a given voltage in the range of 0 to 1.8 V
*
* @returns if setting output voltage was succesful
*/
uint32_t setDACVoltage(float voltage){

	uint16_t dacdata = (uint16_t) round((voltage*4096)/(1.8));
	dacdata = dacdata<<4;
	return am_devices_dac63002_1_set_output(dacdata);
	
}