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

uint16_t integration_time;
uint16_t spectralData[12];          //Why 12? -- Dark is also there
unsigned int spectralDataOld[6];
bool firstChannels;                 // if true -> F1-F6 is measured.

static float spectral_gain = 4.0;

const float responsivity[12] = {0.16, 0.3, 0.38, 0.48, 0.58, 0.66, 0.75, 1, 0.44, 1, 1.6, 4.44};
int8_t adcChMap[6]={as_none,as_none,as_none,as_none,as_none,as_none}; //Internal - Maps the SMUX configuration (ADC0 - ADC5 to the corresponding channels back (CH0 - CH8,  Clear, NIR, Flicker)

as7341_status as7341_enable(){
    // enable power.
    uint8_t error = 0;          // AS7341_REGADDR_ENABLE = 0x80
	uint32_t command = 0x01;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   AS7341_REGADDR_ENABLE,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }
	return error;
}

as7341_status as7341_readStatusRegister(uint32_t *reg){
	
	uint32_t command = STATUS_REG_ADR_AS7341;		// status register
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
    //
    // Send the command sequence to read the status
    //
    if (am_device_command_read(pIom->pIomHandle, ADDRESS_AS7341, 1,
                           command,
                           false, reg, 1))
    {
        return ACKNO_ERROR;
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
    }
    else if (listId == AS7341_SMUX_F7F8) {
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
	}
    else if (listId == AS7341_SMUX_CDNF) {
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
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   0x00,
					   false, regvalbuf32, 20))
    {
        error |= ACKNO_ERROR;
    }

    //i2ctransmit(ADDRESS_AS7341, &regvalbuf[0], 21);
    // write smux conf from ram to smux chain
    regval[0] = 0xAF;                   // AS7341_REGADDR_CFG6 = 0xAF
    regval[1] = 0x10;
	uint32_t command = 0x10;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   0xAF,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }
    //i2ctransmit(ADDRESS_AS7341, &regval[0], 2);
	

    // power on, start smux command
    regval[0] = 0x80;
    regval[1] = 0x11;               // AS7341 ENABLE REGISTER = 0x80
	command = 0x11;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   0x80,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }
	
    //i2ctransmit(ADDRESS_AS7341, &regval[0], 2);
	return error;
}



as7341_status as7341_startMeasurement(bool flicker, bool wait){
    uint8_t regval[2];
    regval[0] = 0x80;               // AS7341 ENABLE REGISTER = 0x80
    regval[1] = 0x03;
    if(flicker)
        regval[1] |= 0x40;
    if(wait)
        regval[1] |= 0x08;

    //FixMe: Looks like the ENABLE Resgister is written wrong and the wait-Flag has no effect here es the wrong bit is written!
    /*
    if (flicker && wait) {
        regval[1] = 0x27;
    } else if (!flicker && wait) {
        regval[1] = 0x07;
    } else if (flicker && !wait) {
        regval[1] = 0x23;
    } else {
        regval[1] = 0x03;
    }
    */
	uint32_t command = regval[1];
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   0x80,
					   false, &command, 1))
    {
        return ACKNO_ERROR;
    }
	return SUCCESS;
    //i2ctransmit(ADDRESS_AS7341, &regval[0], 2);
}
/**
 * Sets wait time in WTIME register (0x83). Wait time can be set in 2,78ms steps from 2,78ms (0x00) to 711ms (0xff)
 *
 */
as7341_status as7341_setWaitTime(uint8_t wtime){
    uint8_t regval[2];
    regval[0] = 0x83;
    regval[1] = wtime;
	
	uint32_t command = regval[1];
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   0x83,
					   false, &command, 1))
    {
        return ACKNO_ERROR;
    }
	return SUCCESS;
    //i2ctransmit(ADDRESS_AS7341, &regval[0], 2);
}
/**
 * Sets integration time. Integration time = (ASTEP + 1) * ( ATIME + 1)*2.78us. Recommended value: 50 ms.
 * ATIME is set to zero, and therefore the time is determined by ASTEP, here it is set to 100 ms
 *
 */
as7341_status as7341_setIntegrationTime(){
    // set ATIME first to 1
    uint8_t regval[3];
	uint8_t error = 0;
    regval[0] = 0x81;
    regval[1] = 0x00;           //1 * 65534 *2,78�s = 182ms
	uint32_t command = regval[1];
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   0x81,
					   false, &command, 1))
    {
        error |= ACKNO_ERROR;
    }

    //i2ctransmit(ADDRESS_AS7341, &regval[0], 2);
    // then set ASTEP. E.G. STEP = 35970 -> integration time = 100ms.
    regval[0] = 0xca;
    //regval[1] = 0x94;           // 0x2A94 = 10900.
    //regval[2] = 0x2A;
    //regval[1] = 0x4F;           // 0x464F = 17999.
    //regval[2] = 0x46;
    //regval[1] = 0x54;           // 0x5554 = 21844.
    //regval[2] = 0x55;
    //regval[1] = 0xF8;           // 0x7FF8 = 32761.
    //regval[2] = 0x7F;
    //regval[1] = 0xFE;           // 0xFFFE = 65534.
    //regval[2] = 0xFF;
    regval[1] = 0x82;          // 0x8C82 = 35970.
    regval[2] = 0x8C;
    //regval[1] = 0x40;          // 0x4640 = 17985.
    //regval[2] = 0x46;
	command = regval[1] + (regval[2]<<8);
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   0xCA,
					   false, &command, 2))
    {
        error |= ACKNO_ERROR;
    }
    //i2ctransmit(ADDRESS_AS7341, &regval[0], 3);
    integration_time = 100;
	return error;
}

as7341_status as7341_setATIME(uint8_t times){
    uint8_t regval[2];
    regval[0] = 0x81;
    regval[1] = times;           //times * ASTEP *2,78�s = Integration time
	uint32_t command = regval[1];
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   0x81,
					   false, &command, 1))
    {
        return ACKNO_ERROR;
    }
	return SUCCESS;
    //i2ctransmit(ADDRESS_AS7341, &regval[0], 2);
}
as7341_status as7341_setASTEP(int i){
    uint8_t regval[3];
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	uint8_t error = 0;
    regval[0] = 0xca;
    switch(i) {
    case 0:
        regval[1] = 0x18;
        regval[2] = 0x01;   // 1*281*2,78�s = 0,7ms
        //i2ctransmit(ADDRESS_AS7341, &regval[0], 3);
        integration_time = 2;
        break;
    case 1:
        regval[1] = 0x31;
        regval[2] = 0x02;   // 1*562*2,78�s = 1,6ms
        //i2ctransmit(ADDRESS_AS7341, &regval[0], 3);
        integration_time = 3;
        break;
    case 2:
        regval[1] = 0x63;
        regval[2] = 0x04;   // 1*1124*2,78�s = 3,125ms
        //i2ctransmit(ADDRESS_AS7341, &regval[0], 3);
        integration_time = 6;
        break;
    case 3:
        regval[1] = 0xC7;
        regval[2] = 0x08;   // 1*2248*2,78�s = 6,25ms
        //i2ctransmit(ADDRESS_AS7341, &regval[0], 3);
        integration_time = 13;
        break;
    case 4:
         regval[1] = 0x8F;
         regval[2] = 0x11;   // 1*4496*2,78�s = 12,5ms
         //i2ctransmit(ADDRESS_AS7341, &regval[0], 3);
         integration_time = 25;
         break;
    case 5:
         regval[1] = 0x1F;
         regval[2] = 0x23;   // 1*8992*2,78�s = 25ms
         //i2ctransmit(ADDRESS_AS7341, &regval[0], 3);
         integration_time = 50;
         break;
    case 6:
         regval[1] = 0x40;
         regval[2] = 0x46;   // 1*17985*2,78�s = 50ms
         //i2ctransmit(ADDRESS_AS7341, &regval[0], 3);
         integration_time = 100;
         break;
    case 7:
         regval[1] = 0x82;
         regval[2] = 0x8C;   // 1*35971*2,78�s = 100ms
         //i2ctransmit(ADDRESS_AS7341, &regval[0], 3);
         integration_time = 182;
         break;
    case 8:
         regval[1] = 0xFE;
         regval[2] = 0xFF;   // 1*65535*2,78�s = 182ms
         //i2ctransmit(ADDRESS_AS7341, &regval[0], 3);
         integration_time = 182;
         break;
    }
	uint32_t command = regval[1] + (regval[2]<<8);
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   0xCA,
					   false, &command, 2))
	{
		error |= ACKNO_ERROR;
	}
	return error;
}


//void as7341_readMeasurementsOld(){
//    uint8_t regval = 0x95; // status register.
//    uint8_t *ptrg = &regval;
//    i2ctransmit(ADDRESS_AS7341, ptrg, 1);
//    i2creceive(ADDRESS_AS7341, 12);
//    int i;
//    for(i = 0; i < 6; i++) {
//        spectralDataOld[i] = I2CRXData[(2*i)];
//        spectralDataOld[i] += (I2CRXData[(2*i+1)]<<8);

//    }
//}

/**
 *
 */
//FixMe: Bad style: result is dependent on variable that is not documented and very unflexible, use the new one
as7341_status as7341_readMeasurements(){
    uint8_t regval = 0x95; // status register.
    uint8_t *ptrg = &regval;
	uint32_t command;
	uint32_t receive[3];

    //
    command = regval;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;

    if (am_device_command_read(pIom->pIomHandle, ADDRESS_AS7341, 1,
                           command,
                           false, receive, 12))
    {
        return ACKNO_ERROR;
    }
    //i2ctransmit(ADDRESS_AS7341, ptrg, 1);
    //i2creceive(ADDRESS_AS7341, 12);
    unsigned int temp;
    //float divider;
    int i;
    if (firstChannels){
        for(i = 0; i < 3; i++) {
            temp = receive[i];
            temp += (receive[i]<<8);
            //divider = spectral_gain * integration_time;
            //spectralData[i] = (temp/divider);       // basic counts
            spectralData[(2*i)] = temp;
			temp = receive[i]<<16;
			temp += receive[i]<<24;
			spectralData[(2*i + 1)] = temp;
        }
    } else {
        for(i = 0; i < 6; i++) {
            temp = receive[i];
            temp += (receive[i]<<8);
            //divider = spectral_gain * integration_time;
            //spectralData[i+6] = (temp/divider);       // basic counts
            spectralData[(2*i+6)] = temp;
			temp = receive[i]<<16;
			temp += receive[i]<<24;
			spectralData[(2*i + 1 + 6)] = temp;
        }
    }
	return SUCCESS;
}

as7341_status as7341_transmitMeasurements(){
    uint8_t regval = 0x95; // status register.
    uint8_t *ptrg = &regval;
	uint32_t command;
	uint32_t receive[3];

    //
    command = regval;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;

    if (am_device_command_read(pIom->pIomHandle, ADDRESS_AS7341, 1,
                           command,
                           false, receive, 12))
    {
        return ACKNO_ERROR;
    }
    //i2ctransmit(ADDRESS_AS7341, ptrg, 1);
    //i2creceive(ADDRESS_AS7341, 12);
    unsigned int temp;
    //float divider;
    int i;
    //--- Read out the ADCs 0 - 5 and map them to the corresponding position in spectralData
        for(i = 0; i < 3; i++) {
            temp = receive[i] & 0xFF;
            temp += (receive[i]>>8) << 8;
            //divider = spectral_gain * integration_time;
            //spectralData[i] = (temp/divider);       // basic counts
            if(adcChMap[2*i]!=as_none){
                spectralData[adcChMap[2*i]] = temp;
            }
			temp = (receive[i]>>16) & 0xFF;
			temp += (receive[i]>>24) << 8;
			if(adcChMap[2*i + 1]!=as_none){
                spectralData[adcChMap[2*i + 1]] = temp;
            }
        }
	return SUCCESS;
}

as7341_status as7341_setGain(uint8_t gain){
    uint8_t regval[2];
    regval[0] = 0xAA;             // CFG1 register.
    regval[1] = gain;           // Gain settings.
	uint32_t command = regval[1];
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   0xAA,
					   false, &command, 1))
    {
        return ACKNO_ERROR;
    }

    //i2ctransmit(ADDRESS_AS7341, &regval[0], 2);

    switch(gain) {
        case 0x00: spectral_gain = 0.5; break;
        case 0x01: spectral_gain = 1.0; break;
        case 0x02: spectral_gain = 2.0; break;
        case 0x03: spectral_gain = 4.0; break;
        case 0x04: spectral_gain = 8.0; break;
        case 0x05: spectral_gain = 16.0; break;
        case 0x06: spectral_gain = 32.0; break;
        case 0x07: spectral_gain = 64.0; break;
        case 0x08: spectral_gain = 128.0; break;
        case 0x09: spectral_gain = 256.0; break;
        case 0x0A: spectral_gain = 512.0; break;
    }
	return SUCCESS;
}

float as7341_getSpectralGain()
{
    return spectral_gain;
}

as7341_status as7341_stopMeasuring(){
    // clear SP_EN bit. Power on bit stays set. And wait enable
    uint8_t regval[] = {AS7341_REGADDR_ENABLE, 0x09};          // AS7341_REGADDR_ENABLE = 0x80
	uint32_t command = regval[1];
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;
	if (am_device_command_write(pIom->pIomHandle, ADDRESS_AS7341, 1,
					   AS7341_REGADDR_ENABLE,
					   false, &command, 1))
    {
        return ACKNO_ERROR;
    }
	return SUCCESS;
    //i2ctransmit(ADDRESS_AS7341, &regval[0], 2);
}

unsigned int as7341_testCommunication(){
    uint8_t g = ID_REG_ADR_AS7341; // status register.
    uint8_t *ptrg = &g;
	uint32_t command = ID_REG_ADR_AS7341;
	uint32_t receive = 0;
	am_devices_iom_as7341_t *pIom = (am_devices_iom_as7341_t *)my_IomdevHdl;

    if (am_device_command_read(pIom->pIomHandle, ADDRESS_AS7341, 1,
                           command,
                           false, &receive, 1))
    {
        return ACKNO_ERROR;
    }
    //i2ctransmit(ADDRESS_AS7341, ptrg, 1);
    //i2creceive(ADDRESS_AS7341, 1);
    if(receive == ID_AS7341_DEC){
        return 1;
    }
    return 0;
}

void as7341_config() {
    as7341_enable();
	//am_hal_flash_delay(FLASH_CYCLES_US(20000));
	timerDelay(20);
    //waitInLPM(20, 0);
    //as7341_writeSMUXmapping(0);
    //waitInLPM(50, 0);
    as7341_setIntegrationTime();        //Sets the integration time to 100ms with ATIME =0
    //waitInLPM(20, 0);
//	am_hal_flash_delay(FLASH_CYCLES_US(20000));
	timerDelay(20);
    as7341_setWaitTime(0xff);
    //waitInLPM(20, 0);
//	am_hal_flash_delay(FLASH_CYCLES_US(20000));
	timerDelay(20);
//    spectral_gain = 256;          // Default value.
//    as7341_setGain(AS7341_AGAIN_1);
//    waitInLPM(20, 0);
}

void as7341_takeResponsivityIntoAccount(){
    int i;
    for(i = 0; i < 12; i++){
        spectralData[i] = spectralData[i]/responsivity[i];
    }
}
