/*!
 *  \file       spi.h
 *
 *  \brief      Provides functions to use the SPI driver
 *
 *  \date       13.04.2023
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
#ifndef SPI_H
#define SPI_H
#include "stdint.h"
#include "stdbool.h"

#define DEVICE_SPI_MAX_DEVICE_NUM 1


//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
    SPI_STATUS_SUCCESS,
    SPI_STATUS_ERROR
} spi_status_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} spi_config_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern uint32_t spi_init();

extern uint32_t spi_term();



extern uint32_t spi_command_write(uint32_t ui32InstrLen, uint64_t ui64Instr,
                        uint32_t *pData, uint32_t ui32NumBytes);


extern uint32_t spi_command_read(uint32_t ui32InstrLen, uint64_t ui64Instr,
                       uint32_t *pData, uint32_t ui32NumBytes);




#endif