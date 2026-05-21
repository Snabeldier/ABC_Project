/*!
 *  \file       dac63002.h
 *
 *  \brief      Provides functions to use the external DAC
 *
 *  \date       15.02.2023
 *
 *  \author     Timm Luhmann (IMTEK)
 */
#ifndef DAC63002_H
#define DAC63002_H
 
 //***** Header Files **********************************************************
#include <stdint.h>
//#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "pins.h"
#include "i2c.h"
#include "gpio.h"
#include "pins.h"
#include "math.h"

//*****************************************************************************
//
// Global definitions for the commands
//
//*****************************************************************************
#define AM_DEVICES_DAC63002_SLAVE_ID          0x4B
#define DEVICE_ID 0x08

#define AM_DEVICES_DAC63002_MAX_DEVICE_NUM    1

#define ADDRESS_DAC1_MARGIN_HIGH 0x01
#define ADDRESS_DAC1_MARGIN_LOW 0x02
#define ADDRESS_DAC1_VOUT_CMP_CONFIG 0x03
#define ADDRESS_DAC1_IOUT_MISC_CONFIG 0x04
#define ADDRESS_DAC1_CMP_MODE_CONFIG 0x05
#define ADDRESS_DAC1_FUNC_CONFIG 0x06

#define ADDRESS_DAC0_MARGIN_HIGH 0x13
#define ADDRESS_DAC0_MARGIN_LOW 0x14
#define ADDRESS_DAC0_VOUT_CMP_CONFIG 0x15
#define ADDRESS_DAC0_IOUT_MISC_CONFIG 0x16
#define ADDRESS_DAC0_CMP_MODE_CONFIG 0x17
#define ADDRESS_DAC0_FUNC_CONFIG 0x18

#define ADDRESS_DAC1_DATA 0x19
#define ADDRESS_DAC0_DATA 0x1C

#define ADDRESS_COMMON_CONFIG 0x1F
#define ADDRESS_COMMON_TRIGGER 0x20
#define ADDRESS_COMMON_DAC_TRIG 0x21
#define ADDRESS_GENERAL_STATUS 0x22
#define ADDRESS_CMP_STATUS 0x23
#define ADDRESS_GPIO_CONFIG 0x24
#define ADDRESS_DEVICE_MODE_CONFIG 0x25
#define ADDRESS_INTERFACE_CONFIG 0x26
#define ADDRESS_SRAM_CONFIG 0x2B
#define ADDRESS_SRAM_DATA 0x2C
#define ADDRESS_BRDCAST_DATA 0x50

#define GAIN_1VREF 0
#define GAIN_1VDD 1
#define GAIN_1_5_INTERNAL 2
#define GAIN_2_INTERNAL 3
#define GAIN_3_INTERNAL 4
#define GAIN_4_INTERAL 5

#define IOUT_0_25 0
#define IOUT_0_50 1
#define IOUT_0_125 2
#define IOUT_0_250 3
#define IOUT_0_m24 4
#define IOUT_0_m48 5
#define IOUT_0_m120 6
#define IOUT_0_m240 7
#define IOUT_m25_25 8
#define IOUT_m50_50 9
#define IOUT_m125_125 10
#define IOUT_m250_250 11

#define WAVE_TRIANGULAR 0
#define WAVE_SAWTOOTH 1
#define WAVE_INVERSESAWTOOTH 2
#define WAVE_SINE 4
#define WAVE_DISABLE 7
#define PHASE0 0
#define PHASE120 1
#define PHASE240 2
#define PHASE90 3

#define CODE_STEP_1 0
#define CODE_STEP_2 1
#define CODE_STEP_3 2
#define CODE_STEP_4 3
#define CODE_STEP_6 4
#define CODE_STEP_8 5
#define CODE_STEP_16 6
#define CODE_STEP_32 7

#define SLEW_RATE_INVALID 0
#define SLEW_RATE_4 1
#define SLEW_RATE_8 2
#define SLEW_RATE_12 3
#define SLEW_RATE_18 4
#define SLEW_RATE_27 5
#define SLEW_RATE_40 6
#define SLEW_RATE_60 7
#define SLEW_RATE_91 8
#define SLEW_RATE_136 9
#define SLEW_RATE_239 10
#define SLEW_RATE_418 11
#define SLEW_RATE_732 12
#define SLEW_RATE_1282 13
#define SLEW_RATE_2563 14
#define SLEW_RATE_5127 15

#define RISE_FALL_SLEW_4 0
#define RISE_FALL_SLEW_12 1
#define RISE_FALL_SLEW_27 2
#define RISE_FALL_SLEW_60 3
#define RISE_FALL_SLEW_136 4
#define RISE_FALL_SLEW_418 5
#define RISE_FALL_SLEW_1282 6
#define RISE_FALL_SLEW_5127 7

#define NO_LATCH_WINDOW_COMP_OUT 0
#define LATCH_WINDOW_COMP_OUT (1<<15)
#define DEVUNLOCKED 0
#define DEVLOCKED (1<<14)
#define DUMP_READ_00 0
#define DUMP_READ_01 (1<<13)
#define DISABLE_INTERNAL_REF 0
#define ENABLE_INTERNAL_REF (1<<12)
#define POWER_UP_VOUT 0
#define POWER_DOWN_VOUT0_10K (1<<10)
#define POWER_DOWN_VOUT0_100k (2<<10)
#define POWER_DOWN_VOUT0_HIZ (3<<10)
#define POWER_DOWN_VOUT1_10K (1<<1)
#define POWER_DOWN_VOUT1_100k (2<<1)
#define POWER_DOWN_VOUT1_HIZ (3<<1)
#define POWER_UP_IOUT 0
#define POWER_DOWN_IOUT0 (1<<9)
#define POWER_DOWN_IOUT 1

#define DEVUNLOCK (0b0101<<12)
#define LOCKDONTCARE 0
#define RESET (0b1010<<8)
#define RESETDONTCARE 0
#define LDACTRIGGER (1<<7)
#define LDACDONTCARE 0
#define CLRTRIGGER (1<<6)
#define CLRDONTCARE 0
#define FAULTDUMPTRIGGER (4<<1)
#define FAULTDUMPDONTCARE 0
#define PROTECTTRIGGER (3<<1)
#define PROTECTDONTCARE 0
#define READONETRIGGER (2<<1)
#define READONEDONTCARE 0
#define NVMPROGTRIGGER (1<<1)
#define NVMPROGDONTCARE 0
#define NVMRELOADTRIGGER 1
#define NVMRELOADDONTCARE 0

#define RESET_CMP_1 (1<<15)
#define RESET_CMP_0 (1<<3)
#define TRIG_MAR_LOW_1 (1<<14)
#define TRIG_MAR_LOW_0 (1<<2)
#define TRIG_MAR_HIGH_1 (1<<13)
#define TRIG_MAR_HIGH_0 (1<<1)
#define START_FUNC_1 (1<<12)
#define START_FUNC_0 1
#define STOP_FUNC 0

#define DIS_MODE_IN (1<<13)
#define PROTECT_CONF_NO_SLEW 0
#define PROTECT_CONF_DAC_CODE (1<<8)
#define PROTECT_CONF_MARGIN_LOW (2<<8)
#define PROTECT_CONF_MARGIN_HIGH (3<<8)


typedef enum
{
    AM_DEVICES_DAC63002_STATUS_SUCCESS,
    AM_DEVICES_DAC63002_STATUS_ERROR
} am_devices_dac63002_status_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} am_devices_dac63002_config_t;




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
am_devices_dac63002_read_id(uint32_t *pDeviceID);




//*****************************************************************************
//
//! @brief Sets the gain for the DAC voltage output mode
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
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
am_devices_dac63002_1_set_gain(uint8_t gain);




//*****************************************************************************
//
//! @brief Disable the comparator functionality on channel 1
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.

//! This function disables the comparator functionality on channel 1 &
//! sets the output pin in push-pull mode for the DAC operation
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_disable_CMP();




//*****************************************************************************
//
//! @brief Sets the output range of the iOut mode
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//! @param iOut 0: 0 to 25µA
//!		   iOut 1: 0 to 50µA
//!		   iOut 2: 0 to 125µA
//!		   iOut 3: 0 to 250µA
//!		   iOut 4: 0 to -24µA
//!		   iOut 5: 0 to -48µA
//!		   iOut 6: 0 to -120µA
//!		   iOut 7: 0 to -240µA
//!		   iOut 8: -25µA to 25µA
//!		   iOut 9: -50µA to 50µA
//!		   iOut 10: -125µA to 125µA
//!		   iOut 11: -250µA to 250µA
//! This function sets the output range in i out mode
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_Iout_Range(uint8_t iOut);




//*****************************************************************************
//
//! @brief Set the DAC1 CMP hysteresis or window function
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//! @param hysteresis 0: no hysteresis
//!					  1: hysteresis using DAC1 Margin High&Low
//!					  2: window comparator mode with DAC1 Margin High&Low setting the window bound
//! This function sets the DAC1 CMP mode either in hysteresis, window or none
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_CMP_CONFIG(uint8_t hysteresis);




//*****************************************************************************
//
//! @brief Clear the DAC1 to Zero/Mid-scale
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//! @param clearSel: 0->set to zero scale
//!					 1->set to mid scale
//! This function disables the comparator functionality on channel 1 &
//! sets the output pin in push-pull mode for the DAC operation
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_clear(uint8_t clearSel);




//*****************************************************************************
//
//! @brief Set the DAC1 broadcast setting
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//! @param broadcast: 0->Dont update DAC1 with broadcast command
//!					  1->Update DAC1 with broadcast command
//! This function disables the comparator functionality on channel 1 &
//! sets the output pin in push-pull mode for the DAC operation
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_broadcast(uint8_t broadcast);




//*****************************************************************************
//
//! @brief Set the DAC1 broadcast setting
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//! @param phase 0: 0°
//!				 1: 120°
//!				 2: 240°
//!				 3: 90°
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
am_devices_dac63002_1_function_generation(uint8_t phase, uint8_t waveform);





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
//!					1: 4µs/step
//!					2: 8µs/step
//!					3: 12µs/step
//!					4: 18µs/step
//!					5: 27.04µs/step
//!					6: 40.48µs/step
//!					7: 60.72µs/step
//!					8: 91.12µs/step
//!					9: 136.72µs/step
//!					10: 239.2µs/step
//!					11: 418.64µs/step
//!					12: 732.56µs/step
//!					13: 1282µs/step
//!					14: 2563.96µs/step
//!					15: 5127.92µs/step
//! This function sets the slew rate for linear mode
//! sets the output pin in push-pull mode for the DAC operation
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_linear_slew(uint8_t codeStep, uint8_t slewRate);


//*****************************************************************************
//
//! @brief Set the DAC1 logarithmic slew rate
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//! @param riseSlew 0: 4µS/step
//!				    1: 12µs/step
//!				    2: 27.04µs/step
//!				    3: 60.72µs/step
//!				    4: 136.72µs/step
//!				    5: 418.64µs/step
//!				    6: 1282µs/step
//!				    7: 5127.92µs/step
//! @param fallSlew 0: 4µS/step
//!				    1: 12µs/step
//!				    2: 27.04µs/step
//!				    3: 60.72µs/step
//!				    4: 136.72µs/step
//!				    5: 418.64µs/step
//!				    6: 1282µs/step
//!				    7: 5127.92µs/step
//! This function sets the logarithmic slew rate for the DAC channel 1
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_logarithmic_slew(uint8_t riseSlew, uint8_t fallSlew);





//*****************************************************************************
//
//! @brief Set the DAC1 data register aka the output value
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//! @param value: 12-bit value for the output
//!
//! This function sets the output value for the DAC
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_dac63002_1_set_output(uint16_t value);




//*****************************************************************************
//
//! @brief Set the DAC1 common config register
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
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
am_devices_dac63002_1_set_common_config(uint32_t data);




//*****************************************************************************
//
//! @brief Set the DAC common trigger register
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
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
am_devices_dac63002_1_set_common_trigger(uint32_t data);



//*****************************************************************************
//
//! @brief Set the DAC trigger register
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
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
am_devices_dac63002_1_set_trigger(uint32_t data);




//*****************************************************************************
//
//! @brief Read the DAC general status register
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
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
am_devices_dac63002_read_general_status(uint32_t *data);



//*****************************************************************************
//
//! @brief Read the DAC general status register
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
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
am_devices_dac63002_read_cmp_status(uint32_t *data);



//*****************************************************************************
//
//! @brief Set device mode config register
//!
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
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
am_devices_dac63002_1_set_device_mode_config(uint32_t data);


uint32_t
am_devices_dac63002_read_output(uint32_t *data);

uint32_t
am_devices_dac63002_write_register(uint32_t data, uint32_t reg);

uint32_t
am_devices_dac63002_read_register(uint32_t *data, uint32_t reg);

uint32_t dataPrepare(uint32_t data);

uint32_t setDACVoltage(float voltage);
#endif