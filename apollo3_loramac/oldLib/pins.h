/*!
 *  \file       pins.h
 *
 *  \brief      This file acts as configuration file for different defines needed in the firmware.
 *
 *  \date       26.01.2023
 *
 *  \author     Timm Luhmann (IMTEK)
 */

#ifndef PINS_H
#define PINS_H

//---------------GPIOs-----------------
#define WP_FRAM 1
#define ON_OFF_3_3 2
#define R_PHOTO_1 4
#define SPIMASTER0CLK 5
#define SPIMASTER0MISO 6
#define SPIMASTER0MOSI 7
#define I2CSCL 8
#define I2CSDA 9
#define ON_OFF_PAM 10
#define ADC_PAM 12
#define ADC_BIAS 13
#define LORA_BUSY 15
#define LORA_IRQ 16
#define LORA_EXTRA2 17			//currently unused
#define ON_OFF_LORA 18
#define LORA_RXSWITCH 19
#define SWO_PIN 22              //used to generate ITM output to the printf terminal
#define ON_OFF_I2C 23
#define R_PHOTO_2 24
#define CS_LORA 25
#define LORA_RESET 26
#define LORA_TXSWITCH 29
#define ADC_CAPACITOR 31
#define ADC_SENSOREXTRA2 32		//currently unused
#define ADC_POWEREXTRA 34		//currently unused
#define AS7341_INT_PIN 35		//currently unused
#define POWER_EXTRA1 36			//currently unused
#define ON_OFF_POWERSENSOR 37
#define ON_OFF_VOLTAGEDIVIDER 38
#define ON_OFF_TEMP 39
#define ON_OFF_PARLAI 40
#define ON_OFF_FRAM 41
#define ON_OFF_DAC 42
#define PWRSAVERTC 44
#define UART0TX 48
#define UART0RX 49

/*
 * These are the switches on the Lora Board PE4259, PAY ATTENTION: The Logic in LAMBDA62-2 Datasheet is vice versa!
 */
#define ANT_SW_PIN      29         /*!< TX_SW_PIN */
#define RX_SW_PIN       19         // RX_SW_PIN


#endif