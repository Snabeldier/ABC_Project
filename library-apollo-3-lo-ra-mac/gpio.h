/*!
 *  \file       gpio.h
 *
 *  \brief      Provides functions to configure GPIOs
 *
 *  \date       02.02.2023
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
#ifndef GPIO_H
#define GPIO_H

#include "am_mcu_apollo.h"

/*
*	used to assign pad functionality
*/
extern const am_hal_gpio_pincfg_t pad0FNCsel;
extern const am_hal_gpio_pincfg_t pad1FNCsel;
extern const am_hal_gpio_pincfg_t pad2FNCsel;
extern const am_hal_gpio_pincfg_t pad3FNCsel;
extern const am_hal_gpio_pincfg_t pad4FNCsel;
extern const am_hal_gpio_pincfg_t pad5FNCsel;
extern const am_hal_gpio_pincfg_t pad6FNCsel;
extern const am_hal_gpio_pincfg_t pad7FNCsel;
extern const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_IOM1_SCL;
extern const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_IOM1_SDA;
extern const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_DSPL1_SDO;
extern const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_DSPL1_SDI;
extern const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_DSPL1_SCL;
extern const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_DSPL1_CS;
extern const am_hal_gpio_pincfg_t gpioLoraIRQ;
extern const am_hal_gpio_pincfg_t gpioPullIntPAR;
extern const am_hal_gpio_pincfg_t gpioLoraInput;

/*!
 * Board GPIO pin names
 */
typedef enum
{
    MCU_PINS,
    IOE_PINS,

    // Not connected
    NC = (int)0xFFFFFFFF
}PinNames;

typedef uint8_t PinModes;
typedef uint8_t PinConfigs;
typedef uint8_t PinTypes;

typedef am_hal_gpio_intdir_e IrqModes;
typedef uint8_t IrqPriorities;
typedef void *GpioIrqHandler;

typedef struct{
	PinNames pin;
	PinModes mode;
	PinConfigs config;
	PinTypes type;
	uint32_t value;
} Gpio_t;

/* initGPIO(void)
 *
 * initializes GPIO-Ports for all boards connected
 *
 * @return success
 *
*/
uint32_t initGPIO(void);


 /*
 * read digital value for given pin
 *
 * @return -1 for fail, otherwise the digital value
 */
uint32_t gpioRead(uint8_t pin);


/*
* write a digital value for given pin
*
* @return success
*/
uint32_t gpioWrite(uint8_t pin, bool out);


 /*
* enable interrupt on given pin
*
* @return success
*/
uint32_t gpioSetInterrupt(uint8_t pin);


 /*
* disable interrupt on given pin
*
* @return success
*/
uint32_t gpioDisableInterrupt(uint8_t pin);


 /*
* clears interrupt for given pin
*
* @return success
*/
uint32_t gpioInterruptClear(uint8_t pin);


 /*
* gives status of interrupts for given bitmap
*
* @return success
*/
uint32_t gpioGetInterruptStatus(uint64_t interruptMap);

/*
 * enables the printf
 */

void enable_printf();

uint32_t gpioSetLoraIRQ(uint8_t pin);

uint32_t gpioInitLora();
#endif