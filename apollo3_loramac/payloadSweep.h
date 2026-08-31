/*!
 * \file      payloadSweep.h
 *
 * \brief     Payload-size sweep for energy characterisation: transmits dummy
 *            LoRaWAN frames from 0 to SWEEP_MAX_PAYLOAD_SIZE bytes
 *            (EU868 DR3/SF9 maximum) and captures INA219 current/voltage
 *            samples for each transmission.
 *
 *            Enable by adding PAYLOAD_SWEEP to the Keil C/C++ Define field.
 */

#include <stdio.h>
#include "../../common/githubVersion.h"
#include "utilities.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "RegionCommon.h"
#include "pins.h"

#include "cli.h"
#include "Commissioning.h"
#include "NvmDataMgmt.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"
#include "LmHandlerMsgDisplay.h"

#ifndef ACTIVE_REGION
#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."
#define ACTIVE_REGION LORAMAC_REGION_EU868
#endif

void payloadSweep(void);
