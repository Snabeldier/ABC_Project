/*!
 * \file      sx126x-board.h
 *
 * \brief     Target board SX126x driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#ifndef __SX126x_BOARD_H__
#define __SX126x_BOARD_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "spi.h"
#include "gpio.h"

#include "pins.h"
#include "sx126x.h"
#include "LoRaMac.h"

/******************************************************************************
 * CONSTANTS for LoRa Send
 *****************************************************************************/
#define MHDR_LEN    1

#define DEVADDR_LEN 4
#define FCTRL_LEN   1
#define FCNT_LEN    2
#define FPORT_LEN   1

#define MIC_LEN     4
#define LORAMAC_PHY_MAXPAYLOAD                      255

/* Define Uplink and Downlink for En- and Decryption
 */
#define DIR_UPLINK                  0
#define DIR_DOWNLINK                1

/*!
 * CMAC/AES Message Integrity Code (MIC) Block B0 size
 */
#define LORAMAC_MIC_BLOCK_B0_SIZE                   16

/*!
 * \brief Initializes the radio I/Os pins interface
 */
void SX126xIoInit( void );

/*!
 * \brief Initializes DIO IRQ handlers
 *
 * \param [IN] irqHandlers Array containing the IRQ callback functions
 */
void SX126xIoIrqInit( DioIrqHandler dioIrq );

/*!
 * \brief De-initializes the radio I/Os pins interface.
 *
 * \remark Useful when going in MCU low power modes
 */
void SX126xIoDeInit( void );

/*!
 * \brief Initializes the TCXO power pin.
 */
void SX126xIoTcxoInit( void );

/*!
 * \brief Initializes RF switch control pins.
 */
void SX126xIoRfSwitchInit( void );

/*!
 * \brief Initializes the radio debug pins.
 */
void SX126xIoDbgInit( void );

/*!
 * \brief HW Reset of the radio
 */
void SX126xReset( void );

/*!
 * \brief Blocking loop to wait while the Busy pin in high
 */
void SX126xWaitOnBusy( void );

/*!
 * \brief Wakes up the radio
 */
void SX126xWakeup( void );

/*!
 * \brief Send a command that write data to the radio
 *
 * \param [in]  opcode        Opcode of the command
 * \param [in]  buffer        Buffer to be send to the radio
 * \param [in]  size          Size of the buffer to send
 */
void SX126xWriteCommand( RadioCommands_t opcode, uint8_t *buffer, uint16_t size );

/*!
 * \brief Send a command that read data from the radio
 *
 * \param [in]  opcode        Opcode of the command
 * \param [out] buffer        Buffer holding data from the radio
 * \param [in]  size          Size of the buffer
 *
 * \retval status Return command radio status
 */
uint8_t SX126xReadCommand( RadioCommands_t opcode, uint8_t *buffer, uint16_t size );

/*!
 * \brief Write a single byte of data to the radio memory
 *
 * \param [in]  address       The address of the first byte to write in the radio
 * \param [in]  value         The data to be written in radio's memory
 */
void SX126xWriteRegister( uint16_t address, uint8_t value );

/*!
 * \brief Read a single byte of data from the radio memory
 *
 * \param [in]  address       The address of the first byte to write in the radio
 *
 * \retval      value         The value of the byte at the given address in radio's memory
 */
uint8_t SX126xReadRegister( uint16_t address );

/*!
 * \brief Sets the radio output power.
 *
 * \param [IN] power Sets the RF output power
 */
void SX126xSetRfTxPower( int8_t power );

/*!
 * \brief Gets the device ID
 *
 * \retval id Connected device ID
 */
uint8_t SX126xGetDeviceId( void );

/*!
 * \brief Initializes the RF Switch I/Os pins interface
 */
void SX126xAntSwOn( void );

/*!
 * \brief De-initializes the RF Switch I/Os pins interface
 *
 * \remark Needed to decrease the power consumption in MCU low power modes
 */
void SX126xAntSwOff( void );


void SX126xRXSwOn( void );

void SX126xRXSwOff( void );
/*!
 * \brief Checks if the given RF frequency is supported by the hardware
 *
 * \param [IN] frequency RF frequency to be checked
 * \retval isSupported [true: supported, false: unsupported]
 */
bool SX126xCheckRfFrequency( uint32_t frequency );

/*!
 * \brief Gets the Defines the time required for the TCXO to wakeup [ms].
 *
 * \retval time Board TCXO wakeup time in ms.
 */
uint32_t SX126xGetBoardTcxoWakeupTime( void );

/*!
 * \brief Gets current state of DIO1 pin state.
 *
 * \retval state DIO1 pin current state.
 */
uint32_t SX126xGetDio1PinState( void );

/*!
 * \brief Gets the current Radio OperationMode variable
 *
 * \retval      RadioOperatingModes_t last operating mode
 */
RadioOperatingModes_t SX126xGetOperatingMode( void );

/*!
 * \brief Sets/Updates the current Radio OperationMode variable.
 *
 * \remark WARNING: This function is only required to reflect the current radio
 *                  operating mode when processing interrupts.
 *
 * \param [in] mode           New operating mode
 */
void SX126xSetOperatingMode( RadioOperatingModes_t mode );

/*!
 * Radio hardware and global parameters
 */
extern SX126x_t SX126x;

#ifdef __cplusplus
}
#endif

LoRaMacStatus_t LoRaMacSendFrame(uint8_t *FRMPayload, uint8_t FRMPayloadLen,
                                 uint8_t *FOpts, uint8_t FOptsLen);

void LoRaMacComputeMic( const uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint32_t *mic );

void LoRaMacPayloadEncrypt( uint8_t *buffer, uint16_t size, const uint8_t *key, uint32_t address, uint8_t dir, uint32_t sequenceCounter, uint8_t *encBuffer );
#endif // __SX126x_BOARD_H__
