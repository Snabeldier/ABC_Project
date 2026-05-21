/*!
 *  \file       as7341.c
 *
 *  \date       02.04.2020
 *
 *  \author     daniel
 *
 *  \modified  	Timm Luhmann(IMTEK)
 */


#include "as7341.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "timing.h"

#include "i2c.h"
#include "am_mcu_apollo.h"

typedef struct
{
    uint32_t                    ui32Module;
    void                        *pIomHandle;
    bool                        bOccupied;
} am_devices_iom_as7341_t;

am_devices_iom_as7341_t gAS7341[AM_DEVICES_AS7341_MAX_DEVICE_NUM];	//one device maximum

bool firstChannels;                 // if true -> F1-F6 is measured.

int8_t adcChMap[6]={as_none,as_none,as_none,as_none,as_none,as_none}; //Internal - Maps the SMUX configuration (ADC0 - ADC5 to the corresponding channels back (CH0 - CH8,  Clear, NIR, Flicker)

 /***************************************************************************//**
 * \brief Powers on the spectral sensor.
 *
 * To operate the device set bit PON = �1� first.
 * Note: When bit is set, internal oscillator is activated, allowing timers and ADC channels to operate. 
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_enable(void){
	uint32_t command = 0x01;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   AS7341_REG_ENABLE,	
					   false, &command, 1))
    {
        return ACKNO_ERROR;
    }
	return SUCCESS;
}

 /***************************************************************************//**
 * \brief Tests communication to the spectral sensor.
 *
 * Reads chip ID (0b001001) from ID Register 0x92 and checks if its matches.
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_testCommunication(void){
	uint32_t command = AS7341_REG_ID;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	uint32_t reg = 0;
		
	if (am_device_command_read(pIom->pIomHandle, AS7341_ADDRESS, 1,
												 command,
												 false, &reg, 1))
	{
			return ACKNO_ERROR;
	}
	
	// Check if ID matches
	if((reg & 0xFC) != (AS7341_CHIP_ID << 2)){
			return ERROR;
	}
	return SUCCESS;
}

 /***************************************************************************//**
 * \brief Enable the measurement of the spectral sensor.
 * 
 * Set start bit SP_EN 0x01 in ENABLE Register 0x80 to enable the measurement of the spectral sensor.
 * 
 * \todo first read reg, second set SP_EN bit, third write the register with SP_EN bit.
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_startMeasurement(void){
	uint32_t command = 0x03;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   AS7341_REG_ENABLE,
					   false, &command, 1))
	{
			return ACKNO_ERROR;
	}
	return SUCCESS;
}

 /***************************************************************************//**
 * \brief Disables the measurement of the spectral sensor.
 * 
 * Reset start bit SP_EN 0x01 in ENABLE Register 0x80 to disable the measurement of the spectral sensor.
 * 
 * \todo first read reg, second remove SP_EN bit, third write the register without SP_EN bit.
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_stopMeasuring(void){
  // clear SP_EN bit. Power on bit stays set. And wait enable
	uint32_t command = 0x09;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   AS7341_REG_ENABLE,
					   false, &command, 1))
    {
        return ACKNO_ERROR;
    }
	return SUCCESS;
}

 /***************************************************************************//**
 * \brief Configures the measurement of the spectral sensor.
 *
 * First power up the spectral sensor. Then set atime, astep and gain.
 *
 * \param [in] uint8_t atime, 0x00 to 0xFF
 * \param [in] uint16_t astep, 0x0000 to 0xFFFE
 * \param [in] as7341_gain_t gain, 0x00 to 0x0A corresponds to 0.5 to 512 gain.
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_config(uint8_t atime, uint16_t astep, as7341_gain_t gain) {
	as7341_status error = 0;
	
  error += as7341_enable();
  error += as7341_setATIME(atime);
	error += as7341_setASTEP(astep); 
  error += as7341_setGain(gain); //0x0A = 512
	
	return error;
}

 /***************************************************************************//**
 * \brief Configures the measurement gain of the spectral sensor.
 *
 * \param [in] as7341_gain_t gain, 0x00 to 0x0A corresponds to 0.5 to 512 gain.
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_setGain(as7341_gain_t gain){
	uint32_t command = (uint32_t) gain;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   AS7341_REG_CFG1,
					   false, &command, 1))
    {
        return ACKNO_ERROR;
    }
	return SUCCESS;
}

 /***************************************************************************//**
 * \brief Gets the measurement gain of the spectral sensor stored in the register.
 *
 * \param [out] uint32_t *reg, 0x00 to 0x0A corresponds to 0.5 to 512 gain.
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_getGain(uint16_t *reg){
	uint32_t command = AS7341_REG_CFG1;		// register to read
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;

	if (am_device_command_read(pIom->pIomHandle, AS7341_ADDRESS, 1,
												 command,
												 false, reg, 1))
	{
			return ACKNO_ERROR;
	}
	return SUCCESS;
}

 /***************************************************************************//**
 * \brief Configures the integration time step size of the spectral sensor.
 *
 * Sets the integration time per step in increments of 2.78�s. The default value is 999.
 * For value astep = n, integration time step size is calculated by 2.78�s x (n+1). 
 *
 * \param [in] uint16_t astep, 0x00 to 0xFFFE
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_setASTEP(uint16_t astep){
	uint32_t command = (uint32_t) astep;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   AS7341_REG_ASTEP,
					   false, &command, 2))
	{
		return ACKNO_ERROR;
	}
	
	return SUCCESS;
}

 /***************************************************************************//**
 * \brief Gets the integration time step size of the spectral sensor stored in the register.
 *
 * \param [out] uint32_t *reg, 0x00 to 0xFFFE
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_getASTEP(uint32_t *reg){
	uint32_t command = AS7341_REG_ASTEP;	
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;

	if (am_device_command_read(pIom->pIomHandle, AS7341_ADDRESS, 1,
												 command,
												 false, reg, 2))
	{
			return ACKNO_ERROR;
	}
	return SUCCESS;
}

 /***************************************************************************//**
 * \brief Configures the number of integration steps of the spectral sensor.
 *
 * Sets the number of integration steps from 1 to 256. 
 * The default value is 0x00.
 * For value atime = n, the integration time is calculated by: ASTEP x (n+1).
 *
 * \param [in] uint16_t atime, 0x00 to 0xFF
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_setATIME(uint8_t atime){         
	uint32_t command = (uint32_t) atime;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					 AS7341_REG_ATIME,
					 false, &command, 1))
	{
			return ACKNO_ERROR;
	}
	return SUCCESS;
}

 /***************************************************************************//**
 * \brief Gets the number of integration steps of the spectral sensor stored in the register.
 *
 * \param [out] uint32_t *reg, 0x00 to 0xFF
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_getATIME(uint32_t *reg){
	uint32_t command = AS7341_REG_ATIME;	
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;

	if (am_device_command_read(pIom->pIomHandle, AS7341_ADDRESS, 1,
												 command,
												 false, reg, 1))
	{
			return ACKNO_ERROR;
	}
	return SUCCESS;
}

 /***************************************************************************//**
 * \brief Configures the integration time of the spectral sensor.
 *
 * Sets the number of integration time (atime) and the step size (astep). 
 * The integration time in �s is calculated as follows: (atime + 1) * (astep + 1) * 2.78
 *
 * \param [in] uint16_t atime, 0x00 to 0xFF
 * \param [in] uint16_t astep, 0x00 to 0xFFFE
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_setTINT(uint8_t atime, uint16_t astep){
	as7341_status error = SUCCESS;
	
	error |= as7341_setATIME(atime);
	error |= as7341_setASTEP(astep);
	
	return error;
}

 /***************************************************************************//**
 * \brief Gets the integration time of the spectral sensor stored in the register.
 *
 * \param [out] double *reg, integration time in milli seconds.
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_getTINT(double *reg){
	uint32_t astep, atime;
	uint32_t error = SUCCESS;
	double result = 0;
	
  error += as7341_getASTEP(&astep);
  error += as7341_getATIME(&atime);
	
	astep = astep & 0xFFFF;
	atime = atime & 0xFFFF;

  *reg = ((double)atime + 1.0) * ((double)astep + 1.0) * 2.78 / 1000.0;
	
	return error;
}

 /***************************************************************************//**
 * \brief Configures the wait time of the spectral sensor.
 *
 * 8-bit value to specify the delay between two consecutive spectral measurements. 
 *
 * \param [in] uint16_t wtime, 0x00 to 0xFF
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_setWaitTime(uint8_t wtime){	
		uint32_t command = wtime;
		am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
		if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
							 AS7341_REG_WTIME,
							 false, &command, 1))
			{
					return ACKNO_ERROR;
			}
		return SUCCESS;
}

 /***************************************************************************//**
 * \brief Convertes the gain in hex-Coding to actual values.
 *
 * 0x00..0x0A to 0.5..512. 0.5 is returened as 0.
 *
 * \param [in] uint16_t *gain, 0x00 to 0x0A
 * \param [out] uint16_t *gain, 0.5 to 512
 *
 * \ret		typedef enum as7341_status: SUCCESS, ERROR, ACKNO_ERROR
 ******************************************************************************/
as7341_status as7341_convertGain(uint16_t *gain)
{
	switch (*gain) {
  case AS7341_AGAIN_05:
    *gain = 0;
    break;
  case AS7341_AGAIN_1:
    *gain = 1;
    break;
  case AS7341_AGAIN_2:
    *gain = 2;
    break;
  case AS7341_AGAIN_4:
    *gain = 4;
    break;
  case AS7341_AGAIN_8:
    *gain = 8;
    break;
  case AS7341_AGAIN_16:
    *gain = 16;
    break;
  case AS7341_AGAIN_32:
    *gain = 32;
    break;
  case AS7341_AGAIN_64:
    *gain = 64;
    break;
  case AS7341_AGAIN_128:
    *gain = 128;
    break;
  case AS7341_AGAIN_256:
    *gain = 256;
    break;
  case AS7341_AGAIN_512:
    *gain = 512;
    break;
  }
	return SUCCESS;
}

/**
 * This function configures the ADC to Diodes mapping according to predefined sets
 */
as7341_status as7341_writeSMUXmapping(uint8_t listId){
    uint8_t regval[2];
    uint8_t regvalbuf[21];
    regvalbuf[0] = 0x00;        // first byte is the address of the ram.
    if(listId == 0x00) {
         // smux configuration for first channels
        // CH1 Violet -> ADC0,      CH2 Deep Blue-> ADC1,       CH3 Blue ->   ADC2,
        // CH4 Cyan ->   ADC3,      CH5 Green ->    ADC4,       CH6 Yellow -> ADC5
         regvalbuf[1]  = 0x30;  //F3/1 --> Blue --> ADC2
         regvalbuf[2]  = 0x01;  //F1/2 --> Violet --> ADC0
         regvalbuf[3]  = 0x00;  //None
         regvalbuf[4]  = 0x00;  //F8/7 --> Red --> Connect to GND
         regvalbuf[5]  = 0x06;  //F6/8 --> Yellow -->  ADC5 (Flicker)
         regvalbuf[6]  = 0x42;  //F2/10 F4/11 --> Deep Blue  | Cyan --> ADC1|ADC3
         regvalbuf[7]  = 0x50;  //F5/13 --> Green --> ADC4
         regvalbuf[8]  = 0x00;  //F7/14 --> Orange --> GND
         regvalbuf[9]  = 0x00;  //C/17  --> Clear --> GND
         regvalbuf[10] = 0x50;  //F5/19 --> Green --> ADC4
         regvalbuf[11] = 0x00;  //F7/20 --> Orange2 --> GND
         regvalbuf[12] = 0x00;  //Not used
         regvalbuf[13] = 0x20;  //F2/25 --> Deep Blue --> ADC1
         regvalbuf[14] = 0x04;  //F4/26 --> Cyan --> ADC3
         regvalbuf[15] = 0x60;  //F8/28 F6/29 --> Deep Red | Yellow --> Gnd| ADC5 (Flicker)      <<-- Error in datasheet: in Fig. 3 smux_config they state F7 instead of F8
         regvalbuf[16] = 0x30;  //F3/31 --> Blue --> ADC2
         regvalbuf[17] = 0x01;  //F1/32 ExInpGPIO --> Violet | None --> ADC0 | GND
         regvalbuf[18] = 0x00;  //ExInpINT Clear/25 --> None | Clear --> GND | GND
         regvalbuf[19] = 0x00;  //DARK --> GND
         regvalbuf[20] = 0x00;  //NIR/38 Flicker/39 --> GND GND
         firstChannels = true;
         int8_t t_adcChMap[6]={as_f1,as_f2,as_f3,as_f4,as_f5,as_f6};
         memcpy(adcChMap, t_adcChMap, 6);
    } else if (listId == 0x01) {
            // smux configuration for rest of channels /
         // CH7 Orange -> ADC0,     CH8 red -> ADC1,    CLEAR -> ADC2,
         // DARK -> ADC3,           NIR -> ADC4,        FLICKER -> ADC5
         regvalbuf[1]  = 0x00;
         regvalbuf[2]  = 0x00;
         regvalbuf[3]  = 0x00;
         regvalbuf[4]  = 0x20;      //F8/7 --> Red --> ADC1
         regvalbuf[5]  = 0x00;
         regvalbuf[6]  = 0x00;
         regvalbuf[7]  = 0x00;
         regvalbuf[8]  = 0x01;      //F7/14 --> Orange --> ADC0
         regvalbuf[9]  = 0x30;      //C/17  --> Clear --> ADC2
         regvalbuf[10] = 0x00;
         regvalbuf[11] = 0x01;      //F7/20 --> Orange2 --> ADC0
         regvalbuf[12] = 0x00;
         regvalbuf[13] = 0x00;
         regvalbuf[14] = 0x00;
         regvalbuf[15] = 0x02;      //F8/28 F6/29 --> Deep Red | Yellow --> ADC1| GND      <<-- Error in datasheet: in Fig. 3 smux_config they state F7 instead of F8
         regvalbuf[16] = 0x00;
         regvalbuf[17] = 0x00;
         regvalbuf[18] = 0x30;      //ExInpINT Clear/25 --> None | Clear --> GND | ADC2
         regvalbuf[19] = 0x40;      //DARK --> ADC3
         regvalbuf[20] = 0x65;      //NIR/38 Flicker/39 --> ADC4 ADC5
         firstChannels = false;
         int8_t t_adcChMap[6]={as_f7,as_f8,as_clear,as_dark,as_nir,as_flicker};
         memcpy(adcChMap, t_adcChMap, 6);

    } else if (listId == 0x02) {
        // F2 -> ADC0, F3 -> ADC1, F5 -> ADC2, F7 -> ADC3, F8-> ADC4, CLEAR -> ADC5
         regvalbuf[1]  = 0x20;
         regvalbuf[2]  = 0x00;
         regvalbuf[3]  = 0x00;
         regvalbuf[4]  = 0x50;
         regvalbuf[5]  = 0x00;
         regvalbuf[6]  = 0x10;
         regvalbuf[7]  = 0x30;
         regvalbuf[8]  = 0x04;
         regvalbuf[9]  = 0x60;
         regvalbuf[10] = 0x30;
         regvalbuf[11] = 0x04;
         regvalbuf[12] = 0x00;
         regvalbuf[13] = 0x10;
         regvalbuf[14] = 0x00;
         regvalbuf[15] = 0x05;
         regvalbuf[16] = 0x20;
         regvalbuf[17] = 0x00;
         regvalbuf[18] = 0x60;
         regvalbuf[19] = 0x00;
         regvalbuf[20] = 0x00;
    } else if (listId == AS7341_SMUX_F7F8) {
				 // smux configuration for rest of channels /
				 // CH7 Orange -> ADC0,     CH8 red -> ADC1,
				 regvalbuf[1]  = 0x00;
				 regvalbuf[2]  = 0x00;
				 regvalbuf[3]  = 0x00;
				 regvalbuf[4]  = 0x20;      //F8/7 --> Red --> ADC1
				 regvalbuf[5]  = 0x00;
				 regvalbuf[6]  = 0x00;
				 regvalbuf[7]  = 0x00;
				 regvalbuf[8]  = 0x01;      //F7/14 --> Orange --> ADC0
				 regvalbuf[9]  = 0x00;      //C/17  --> Clear --> GND
				 regvalbuf[10] = 0x00;
				 regvalbuf[11] = 0x01;      //F7/20 --> Orange2 --> ADC0
				 regvalbuf[12] = 0x00;
				 regvalbuf[13] = 0x00;
				 regvalbuf[14] = 0x00;
				 regvalbuf[15] = 0x02;      //F8/28 F6/29 --> Deep Red | Yellow --> ADC1| GND      <<-- Error in datasheet: in Fig. 3 smux_config they state F7 instead of F8
				 regvalbuf[16] = 0x00;
				 regvalbuf[17] = 0x00;
				 regvalbuf[18] = 0x00;      //ExInpINT Clear/25 --> None | Clear --> GND | ADC2
				 regvalbuf[19] = 0x00;      //DARK --> ADC3
				 regvalbuf[20] = 0x00;      //NIR/38 Flicker/39 --> ADC4 ADC5
				 firstChannels = false;     //Strictly speaking, this is wrong
				 int8_t t_adcChMap[6]={as_f7,as_f8,as_none,as_none,as_none,as_none};
				 memcpy(adcChMap, t_adcChMap, 6);
		} else if (listId == AS7341_SMUX_CDNF) {
				 // smux configuration for rest of channels /
				 // CLEAR -> ADC2,
				 // DARK -> ADC3,           NIR -> ADC4,        FLICKER -> ADC5
				 regvalbuf[1]  = 0x00;
				 regvalbuf[2]  = 0x00;
				 regvalbuf[3]  = 0x00;
				 regvalbuf[4]  = 0x00;      //F8/7 --> Red --> GND
				 regvalbuf[5]  = 0x00;
				 regvalbuf[6]  = 0x00;
				 regvalbuf[7]  = 0x00;
				 regvalbuf[8]  = 0x00;      //F7/14 --> Orange --> GND
				 regvalbuf[9]  = 0x30;      //C/17  --> Clear --> ADC2
				 regvalbuf[10] = 0x00;
				 regvalbuf[11] = 0x00;      //F7/20 --> Orange2 --> GND
				 regvalbuf[12] = 0x00;
				 regvalbuf[13] = 0x00;
				 regvalbuf[14] = 0x00;
				 regvalbuf[15] = 0x00;      //F8/28 F6/29 --> Deep Red | Yellow --> GND| GND      <<-- Error in datasheet: in Fig. 3 smux_config they state F7 instead of F8
				 regvalbuf[16] = 0x00;
				 regvalbuf[17] = 0x00;
				 regvalbuf[18] = 0x30;      //ExInpINT Clear/25 --> None | Clear --> GND | ADC2
				 regvalbuf[19] = 0x40;      //DARK --> ADC3
				 regvalbuf[20] = 0x65;      //NIR/38 Flicker/39 --> ADC4 ADC5
				 firstChannels = false;
				 int8_t t_adcChMap[6]={as_none,as_none,as_clear,as_dark,as_nir,as_flicker};
				 memcpy(adcChMap, t_adcChMap, 6);
		}

		uint8_t error = 0;          //
		uint32_t regvalbuf32[5] = {0};
		uint8_t i;
		
		for(i = 0; i < 20; i++){		//adopting uint8 array to uint32 array
			regvalbuf32[i/4] |= (regvalbuf[i+1]<<(8*(i%4)));
		}
		
		am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
		if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
							 0x00,
							 false, regvalbuf32, 20))
		{
				error |= ACKNO_ERROR;
		}

    // write smux conf from ram to smux chain
		uint32_t command = 0x10;
		if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
							 AS7341_REG_CFG6,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }
	

    // power on, start smux command
		command = 0x11;
		if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
							 AS7341_REG_ENABLE,
							 false, &command, 1))
		{
				error |= ACKNO_ERROR;
		}
	
	return error;
}






as7341_status as7341_setInterruptMode(){
	uint8_t regval[3];
	uint8_t error = 0;
	uint32_t receive[3];
    regval[0] = 0x70;
    regval[1] = 0x00;           //set Sync to interrupt pin, SPM mode
	uint32_t command = regval[1];
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   0x70,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }
    if (am_device_command_read(pIom->pIomHandle, AS7341_ADDRESS, 1,
                           0x80,
                           false, receive, 1))
    {
        return ACKNO_ERROR;
    }
    regval[0] = 0xBE;
    regval[1] = 0x06;           //GPIO input enable, GPIO input
	command = regval[1];
	pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   0xBE,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }	
	regval[0] = 0xF9;
	regval[1] = 0x05;		//enable interrupt for FIFO and SMUX
	command = regval[1];
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   0xF9,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }
	regval[0] = 0xB2;
	regval[1] = 0x10;		//enable interrupt for FIFO and SMUX
	command = regval[1];
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   0xB2,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }
	regval[0] = 0xFC;
	regval[1] = 0x7E;		//enable FIFO storage for all 6 channels
	command = regval[1];
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   0xFC,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }	
	regval[0] = 0xB1;
	regval[1] = 0x48;		//enable FIFO threshold lvl is 4
	command = regval[1];
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   0xB1,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }	
	regval[0] = 0xFA;
	regval[1] = 0x02;		//Clear fifo
	command = regval[1];
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   0xFA,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }	
	
	return error;
}




as7341_status as7341_transmitMeasurements(uint16_t *data){
	uint32_t receive[3];
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;

	if (am_device_command_read(pIom->pIomHandle, AS7341_ADDRESS, 1,
												 AS7341_REG_STATUS,
												 false, receive, 12))
	{
			return ACKNO_ERROR;
	}
	
	uint32_t temp;
	
	//--- Read out the ADCs 0 - 5 and map them to the corresponding position in spectralData
	for(uint8_t i = 0; i < 3; i++) {
		
		temp = receive[i] & 0xFF;
		temp += (receive[i]>>8) << 8;
		if(adcChMap[2*i]!=as_none){
				data[adcChMap[2*i]] = temp;
		}
		
		temp = (receive[i]>>16) & 0xFF;
		temp += (receive[i]>>24) << 8;
		if(adcChMap[2*i + 1]!=as_none){
							data[adcChMap[2*i + 1]] = temp;
					}
			}
	
	return SUCCESS;
}

as7341_status as7341_readRegister(uint32_t reg, uint32_t *data){
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;

	if (am_device_command_read(pIom->pIomHandle, AS7341_ADDRESS, 1,
												 reg,
												 false, data, 1))
	{
			return ACKNO_ERROR;
	}
	return SUCCESS;
}

as7341_status as7341_clearInterrupt(uint8_t flags){
	uint32_t command = flags;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, AS7341_ADDRESS, 1,
					   0x93,	
					   false, &command, 1))
    {
        return ACKNO_ERROR;
    }
	return SUCCESS;
	
}



// todo: STOP condition
as7341_status as7341_delayForData(void)
{
	uint32_t command = AS7341_REG_STATUS2;		// register to read
	uint32_t reg = 0;
	uint32_t ready = 0;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	
	while(!ready)
	{
    //
    // Send the command sequence to read the status
    //
    if (am_device_command_read(pIom->pIomHandle, AS7341_ADDRESS, 1,
                           command,
                           false, &reg, 1))
    {
        return ACKNO_ERROR;
    }
		
		if((reg & 0x40))
		{
			ready = 1;
			return SUCCESS;
		}
		
		timerDelay(10);
		
	}
	return SUCCESS;
}


// todo: STOP condition
as7341_status as7341_delayForSMUX(void)
{
	uint32_t command = AS7341_REG_ENABLE;		// register to read
	uint32_t reg = 0;
	uint32_t ready = 0;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	
	while(!ready)
	{
    //
    // Send the command sequence to read the status
    //
    if (am_device_command_read(pIom->pIomHandle, AS7341_ADDRESS, 1,
                           command,
                           false, &reg, 1))
    {
        return ACKNO_ERROR;
    }
		
		if(!(reg & 0x10))
		{
			ready = 1;
			return SUCCESS;
		}
		
		timerDelay(10);
		
	}
	return SUCCESS;
}