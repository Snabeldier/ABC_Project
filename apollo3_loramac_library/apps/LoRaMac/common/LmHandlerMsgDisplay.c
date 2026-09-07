/*!
 * \file      LmHandlerMsgDisplay.h
 *
 * \brief     Common set of functions to display default messages from
 *            LoRaMacHandler.
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
 *              (C)2013-2019 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "utilities.h"
#include "timer.h"

#include "LmHandlerMsgDisplay.h"

#include "am_util_stdio.h"	//required for printf operations on console

void PrintHexBuffer( uint8_t *buffer, uint8_t size )
{
    (void)buffer; (void)size;
}

void DisplayNvmDataChange( LmHandlerNvmContextStates_t state, uint16_t size )
{
    (void)state; (void)size;
}

void DisplayNetworkParametersUpdate( CommissioningParams_t *commissioningParams )
{
    (void)commissioningParams;
}

void DisplayMacMcpsRequestUpdate( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn )
{
    (void)status; (void)mcpsReq; (void)nextTxIn;
}

void DisplayMacMlmeRequestUpdate( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn )
{
    (void)status; (void)mlmeReq; (void)nextTxIn;
}

void DisplayJoinRequestUpdate( LmHandlerJoinParams_t *params )
{
    (void)params;
}

void DisplayTxUpdate( LmHandlerTxParams_t *params )
{
    (void)params;
}

void DisplayRxUpdate( LmHandlerAppData_t *appData, LmHandlerRxParams_t *params )
{
    (void)appData; (void)params;
}

void DisplayBeaconUpdate( LoRaMacHandlerBeaconParams_t *params )
{
    (void)params;
}

void DisplayClassUpdate( DeviceClass_t deviceClass )
{
    (void)deviceClass;
}

void DisplayAppInfo( const char* appName, const Version_t* appVersion, const Version_t* gitHubVersion )
{
    (void)appName; (void)appVersion; (void)gitHubVersion;
}
