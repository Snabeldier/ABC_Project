/***************************************************************************//**
 *  \file       taskHandler.c
 *
 *  \brief      Provides multiple functions for tasks used in the main.
 *
 *  \date       06.03.2024
 *
 *  \author     Uttunga Shinde ( Uni Freiburg IMTEK )
 *
 *  \author     Johannes Klueppel ( Uni Freiburg IMTEK )
 ******************************************************************************/
 
 #include "taskHandler.h"
 
 /******************************************************************************
 * VARIABLES
 *****************************************************************************/

static bool parInitialized = false;
static as7341_gain_t new_gain = AS7341_AGAIN_512;
/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/
uint32_t SpectrometerMeasurement(uint16_t *data);

/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/