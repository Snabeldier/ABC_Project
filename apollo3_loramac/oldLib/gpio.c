/*!
 *  \file       gpio.c
 *
 *  \brief      Provides functions to configure GPIOs
 *
 *  \date       02.02.2023
 *
 *  \author     Timm Luhmann (IMTEK)
 */
//***** Header Files **********************************************************
//#include <stdint.h>
//#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "pins.h"
#include "gpio.h"
#include "am_util_stdio.h"

//***** Defines ***************************************************************

//*****************************************************************************
// Initialize GPIO
//*****************************************************************************

//*****************************************************************************
//  Define some common GPIO configurations.
//*****************************************************************************
const am_hal_gpio_pincfg_t pad0FNCsel = 
{
    .uFuncSel       = 0,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,

};

const am_hal_gpio_pincfg_t pad1FNCsel =
{
    .uFuncSel       = 1,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,

};
const am_hal_gpio_pincfg_t pad2FNCsel = 
{
    .uFuncSel       = 2,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,

};
const am_hal_gpio_pincfg_t pad3FNCsel = 
{
    .uFuncSel       = 3,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,

};
const am_hal_gpio_pincfg_t pad4FNCsel = 
{
    .uFuncSel       = 4,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,

};
const am_hal_gpio_pincfg_t pad5FNCsel =
{
    .uFuncSel       = 5,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,

};
const am_hal_gpio_pincfg_t pad6FNCsel =
{
    .uFuncSel       = 6,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,

};
const am_hal_gpio_pincfg_t pad7FNCsel =
{
    .uFuncSel       = 7,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,

};

	
//*****************************************************************************
//
//  IOM1_SCL pin: I/O Master 1 I2C clock signal.
//
//*****************************************************************************
const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_IOM1_SCL =
{
    .uFuncSel            = AM_HAL_PIN_8_M1SCL,
    .ePullup             = AM_HAL_GPIO_PIN_PULLUP_6K,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .eGPOutcfg           = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
    .uIOMnum             = 1
};

//*****************************************************************************
//
//  IOM1_SDA pin: I/O Master 1 I2C data signal.
//
//*****************************************************************************
const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_IOM1_SDA =
{
    .uFuncSel            = AM_HAL_PIN_9_M1SDAWIR3,
    .ePullup             = AM_HAL_GPIO_PIN_PULLUP_6K,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .eGPOutcfg           = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
    .uIOMnum             = 1
};

//*****************************************************************************
//
//  DSPL1_CS pin: Display DSPL1 I/O Master 0 chip select.
//
//*****************************************************************************
const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_DSPL1_CS =
{
    .uFuncSel            = AM_HAL_PIN_25_NCE25,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .eGPOutcfg           = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
    .eGPInput            = AM_HAL_GPIO_PIN_INPUT_NONE,
    .eIntDir             = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .uIOMnum             = 0,
    .uNCE                = 2,
    .eCEpol              = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW
};

//*****************************************************************************
//
//  DSPL1_SCL pin: Display DSPL1 I/O Master 0 SPI clock signal.
//
//*****************************************************************************
const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_DSPL1_SCL =
{
    .uFuncSel            = AM_HAL_PIN_5_M0SCK,
    .ePullup             = AM_HAL_GPIO_PIN_PULLUP_1_5K,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .eGPOutcfg           = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
    .uIOMnum             = 0
};

//*****************************************************************************
//
//  DSPL1_SDI pin: Display DSPL1 I/O Master 0 SPI data in (MOSI) signal.
//
//*****************************************************************************
const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_DSPL1_SDI =
{
    .uFuncSel            = AM_HAL_PIN_7_M0MOSI,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .uIOMnum             = 0
};

//*****************************************************************************
//
//  DSPL1_SDO pin: Display DSPL1 I/O Master 0 SPI data out (MISO) signal.
//
//*****************************************************************************
const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_DSPL1_SDO =
{
    .uFuncSel            = AM_HAL_PIN_6_M0MISO,
    .uIOMnum             = 0
};


const am_hal_gpio_pincfg_t gpioLoraIRQ =
{
    .uFuncSel            = 3,
	.eGPOutcfg      	= AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .eGPInput       	= AM_HAL_GPIO_PIN_INPUT_ENABLE,
    .eGPRdZero     		= AM_HAL_GPIO_PIN_RDZERO_READPIN,
	.eIntDir			= AM_HAL_GPIO_PIN_INTDIR_LO2HI
};

//const am_hal_gpio_pincfg_t gpioLoraInput =
//{
//    .uFuncSel            = 3,
//	.eGPOutcfg      	= AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
//    .eGPInput       	= AM_HAL_GPIO_PIN_INPUT_ENABLE,
//    .eGPRdZero      	= AM_HAL_GPIO_PIN_RDZERO_READPIN,
//};

/* initGPIO(void)
 *
 * initializes GPIO-Ports for all boards connected
 *
 * @return success
 *
*/
uint32_t initGPIO(void){
	uint32_t success = 0;
	//GPIOs used as output
	success += am_hal_gpio_pinconfig(WP_FRAM,g_AM_HAL_GPIO_OUTPUT);            //Pin WP FRAM output
	success += am_hal_gpio_pinconfig(ON_OFF_3_3,g_AM_HAL_GPIO_OUTPUT);             //Pin turn 3.3V generation on/off output
	success += am_hal_gpio_pinconfig(R_PHOTO_1,g_AM_HAL_GPIO_OUTPUT);            //Pin R_Photo_1 output
	success += am_hal_gpio_pinconfig(ON_OFF_PAM,g_AM_HAL_GPIO_OUTPUT);           //Pin to switch PAM on/off
	success += am_hal_gpio_pinconfig(ON_OFF_LORA,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch LoRa module on/off
	success += am_hal_gpio_pinconfig(ON_OFF_I2C,g_AM_HAL_GPIO_OUTPUT);           //Pin to switch I2C pull-ups on/off
	success += am_hal_gpio_pinconfig(R_PHOTO_2,g_AM_HAL_GPIO_OUTPUT);            //Pin R_Photo_2 output
	success += am_hal_gpio_pinconfig(ON_OFF_POWERSENSOR,g_AM_HAL_GPIO_OUTPUT);   //Pin to switch Powersensor on/off
	success += am_hal_gpio_pinconfig(ON_OFF_VOLTAGEDIVIDER,g_AM_HAL_GPIO_OUTPUT);//Pin to switch the voltagedivider @ cap on/off
	success += am_hal_gpio_pinconfig(ON_OFF_TEMP,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch Tempsensor on/off
	success += am_hal_gpio_pinconfig(ON_OFF_PARLAI,g_AM_HAL_GPIO_OUTPUT);        //Pin to switch AS7341 on/off
	success += am_hal_gpio_pinconfig(ON_OFF_FRAM,g_AM_HAL_GPIO_OUTPUT);          //Pin to switch FRAM on/off
	success += am_hal_gpio_pinconfig(ON_OFF_DAC,g_AM_HAL_GPIO_OUTPUT);           //Pin to switch DAC on/off
	success += am_hal_gpio_pinconfig(CS_LORA,g_AM_HAL_GPIO_OUTPUT);              //Chip select LoRa
	success += am_hal_gpio_pinconfig(LORA_RXSWITCH,g_AM_HAL_GPIO_OUTPUT);        //Enable RX path on LoRa module
	success += am_hal_gpio_pinconfig(LORA_TXSWITCH,g_AM_HAL_GPIO_OUTPUT);        //Enable TX path on LoRa module
	success += am_hal_gpio_pinconfig(LORA_RESET,g_AM_HAL_GPIO_OUTPUT);           //Reset trigger LoRa module
	
	//GPIOs used as input
	success += am_hal_gpio_pinconfig(LORA_BUSY,g_AM_HAL_GPIO_INPUT);			//used for LoRa
	success += am_hal_gpio_pinconfig(LORA_IRQ,gpioLoraIRQ);			//used for LoRa
	
	//GPIOs used as analog input
	success += am_hal_gpio_pinconfig(ADC_CAPACITOR,pad0FNCsel);                //Capacitor Pin set to ADC mode
	success += am_hal_gpio_pinconfig(ADC_PAM,pad0FNCsel);                      //Pam Pin set to ADC mode
	success += am_hal_gpio_pinconfig(ADC_BIAS,pad0FNCsel);    				  //Vdd/2 signal from Pam measurement
	success += am_hal_gpio_pinconfig(ADC_SENSOREXTRA1,pad0FNCsel);			  //currently unused
	success += am_hal_gpio_pinconfig(ADC_SENSOREXTRA2,pad0FNCsel);			  //currently unused
	success += am_hal_gpio_pinconfig(ADC_POWEREXTRA,pad0FNCsel);				  //currently unused
  
	//I2C & SPI pins
	success += am_hal_gpio_pinconfig(I2CSCL,pad0FNCsel);						//select M1SCL
	success += am_hal_gpio_pinconfig(I2CSDA,pad0FNCsel);						//select M1SDAWIR3
	success += am_hal_gpio_pinconfig(SPIMASTER0CLK,pad1FNCsel);				//select M0SCK
	success += am_hal_gpio_pinconfig(SPIMASTER0MISO,pad1FNCsel);				//select M0MISO
	success += am_hal_gpio_pinconfig(SPIMASTER0MOSI,pad1FNCsel);				//select M0MOSI
	
	//UART
	success += am_hal_gpio_pinconfig(UART0TX,pad0FNCsel);					//select UART0TX
	success += am_hal_gpio_pinconfig(UART0RX,pad0FNCsel);					//select UART0RX
	
	return success;
  }    

  /*
  * @brief read digital value for given pin
  *
  * @return -1 for fail, otherwise the digital value
  */
uint32_t gpioRead(uint8_t pin){
	uint32_t readVal;
	if(am_hal_gpio_isinput(pin)){
		uint32_t success = am_hal_gpio_state_read(pin,AM_HAL_GPIO_INPUT_READ,&readVal);
		return readVal;
	}
	else{
		return -1;
	}

}

//uint32_t gpioSetLoraIRQ(uint8_t pin){
//	uint32_t success = 0;
//	success += am_hal_gpio_pinconfig(pin,gpioLoraIRQ);
//	return success;
//}

//uint32_t gpioInitLora(){
//	uint32_t success = 0;
//	success += am_hal_gpio_pinconfig(LORA_BUSY,gpioLoraInput);
//	return success;
//}
/*
* @brief write a digital value for given pin
*
* @return success
*/
uint32_t gpioWrite(uint8_t pin, bool out){
	uint32_t success;
	if(out){
		success = am_hal_gpio_state_write(pin,AM_HAL_GPIO_OUTPUT_SET);		//set output to 1
	}
	else{
		success = am_hal_gpio_state_write(pin,AM_HAL_GPIO_OUTPUT_CLEAR);	//Set output to 0
	}
	return success;	
}
 /*
* @brief enable interrupt on given pin
*
* @return success
*/
uint32_t gpioSetInterrupt(uint8_t pin){
	return am_hal_gpio_interrupt_enable(pin);
}
 /*
* @brief disable interrupt on given pin
*
* @return success
*/
uint32_t gpioDisableInterrupt(uint8_t pin){
	return am_hal_gpio_interrupt_disable(pin);
}
 /*
* @brief clears interrupt for given pin
*
* @return success
*/
uint32_t gpioInterruptClear(uint8_t pin){
	return am_hal_gpio_interrupt_clear(pin);
}
 /*
* @brief gives status of interrupts for given bitmap
*
* @return success
*/
uint32_t gpioGetInterruptStatus(uint64_t interruptMap){
	return am_hal_gpio_interrupt_status_get(1,&interruptMap);
}

//for registering a interrupt function use hal function

void
am_gpio_isr(void)
{
	uint64_t isFired;
	uint32_t status = 0;
	//get the origin of the interrupt
	am_hal_gpio_interrupt_status_get(true,&isFired);
	//and then let the service function do the interrupt service routine handling
	status = am_hal_gpio_interrupt_service(isFired);

    am_hal_gpio_interrupt_clear(isFired);
}

/*
* @brief This function is required to use the printf to terminal functionality in Keil Uvision/Jlink SWO viewer
*		 It configures the ITM to use the SWO pin for printf calls
* 
*/
void enable_printf(){
	am_hal_tpiu_config_t TPIUcfg;
	//Enable the ITM and SWO pin
	am_hal_itm_enable();
	//
    // Enable the ITM and TPIU
    // Set the BAUD clock for 1M
    //
    TPIUcfg.ui32SetItmBaud = AM_HAL_TPIU_BAUD_1M;
    am_hal_tpiu_enable(&TPIUcfg);
    am_hal_gpio_pinconfig(SWO_PIN, pad7FNCsel);

    //
    // Attach the ITM to the STDIO driver.
    //
    am_util_stdio_printf_init(am_hal_itm_print);	
}
