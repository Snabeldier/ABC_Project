/*!
 * \file      periodicUplink.c
 *
 * \brief     Performs a periodic uplink
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
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */


#include <stdio.h>
#include "../../common/githubVersion.h"
#include "utilities.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "RegionCommon.h"
#include "transmitConfig.h"
#include "pins.h"
#include "am1805.h"
#include "taskHandler.h"

#include "cli.h"
#include "Commissioning.h"
#include "NvmDataMgmt.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"
//#include "CayenneLpp.h"
#include "JalapenosLpp.h"
#include "LmHandlerMsgDisplay.h"


#ifndef ACTIVE_REGION

#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."

#define ACTIVE_REGION LORAMAC_REGION_EU868

#endif


extern volatile bool requireTimeRequest;



void periodicUplink(void);