/*!
 *  \file       transmitConfig.h
 *
 *  \date       07.08.2018
 *
 *  \author     baeumker ( Uni Freiburg IMTEK )
 *
 *  \author     Johannes Klueppel (Uni Freiburg IMTEK)
 *
 *  \brief      Transmit parameters for the radio module.
 */
//#include "def.h"
#include "radio.h"

#ifndef TRANSMITTER_CUSTOM_H_
#define TRANSMITTER_CUSTOM_H_

//#include "mcuspecific/mcu.h"    //needed for int/uint definition
//#include "mcuspecific/uart.h"

//----------------- Settings for TX and RX Config ---------------------------------------------------
#define LORA_MODEM                  MODEM_LORA
#define LORA_TX_OUTPUT_POWER        14         // TX output_power[dBm]
#define LORA_FDEV                   0          // LoRa: 0
#define LORA_BANDWIDTH              0           // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] -> const RadioLoRaBandwidths_t Bandwidths[] in radio.c
#define LORA_SPREADING_FACTOR       LORA_SF9//0x07  // also called Datarate [SF7..SF12], 9 used before
#define LORA_RX2_SPREADING_FACTOR   0x09        // also called Datarate
#define LORA_CODINGRATE             1           // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_BANDWIDTHAFC           0           // LoRa: N/A ( set to 0 )
#define LORA_PREAMBLE_LENGTH_TX     6           // SX126x: Default is 12
#define LORA_PREAMBLE_LENGTH_RX     32          // SX126x: For receiving (when unknown: longer is better)
#define LORA_SYMBOL_TIMEOUT         0           // Set to 0, otherwise leads to Rx Timeout
#define LORA_FIX_LENGTH             0           // [0: variable, 1: fixed]
#define LORA_PAYLOAD_LENGTH         0           // Sets payload length when fixed length is used
#define LORA_CRC_TX                 1           // [0: OFF, 1: ON]
#define LORA_CRC_RX                 0           // [0: OFF, 1: ON]
#define LORA_FREQ_HOP               0           // [0: OFF, 1: ON]
#define LORA_HOP_PERIOD             0           // Number of symbols
#define LORA_IQ_INVERSION_TX        0           // [0: not inverted, 1: inverted]
#define LORA_IQ_INVERSION_RX        1           // [0: not inverted, 1: inverted]
#define LORA_RX_CONTINOUS           false       // [false: single mode, true: continuous mode]
#define LORA_TX_TIMEOUT_VALUE       70000u      // Timeout Duration: x*15.625 uS (100000)==>1.5s
#define LORA_RX1_TIMEOUT_VALUE      200000u     // Timeout of receiving window = Timeout_value * 15.625 us -> 64000 * 15.625us = 1s
#define LORA_RX2_TIMEOUT_VALUE      600000u     // 384000 * 15.625us = 6s
#define LORA_RX_WINDOWTIMER1        100         // 0,1s
#define LORA_RX_WINDOWTIMER2        1500        // 1,5s
#define LORA_PORT                   1

// ------------- Timeout Value for uC Timer ----------
#define TIMER_RX_TIMEOUT_VALUE      2500u     // Value / 1024Hz = time in sec.

// ------------- Frequency Definition ---------
#define LORA_RF_FREQUENCY               868100000ul // Hz used to to the set the Calibration Image
#define LORA_RF_FREQUENCY_RX2_DEFAULT   869525000ul // Hz


#define BOARD_TCXO_WAKEUP_TIME  20                  // Time for wakeup of the TCXO * 15.625 uS - This can take up to 1 ms


#endif /* TRANSMITTER_CUSTOM_H_ */
