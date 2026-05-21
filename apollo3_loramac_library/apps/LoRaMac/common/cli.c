/*!
 * \file      cli.h
 *
 * \brief     Command Line Interface handling implementation
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
 *              (C)2013-2020 Semtech
 *
 * \endcode
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "NvmDataMgmt.h"
#include "cli.h"

#include "am_util_stdio.h"	//required for printf operations on console

void CliProcess( Uart_t* uart )
{
    uint8_t data = 0;

    if( UartGetChar( uart, &data ) == 0 )
    {
        if( data == '\x1B' )
        { // Escape character has been received
            am_util_stdio_printf( "ESC + " );
            while( UartGetChar( uart, &data ) != 0 )
            {
            }
            am_util_stdio_printf( "%c\n", data );
            if( data == 'N' )
            { // N character has been received
                data = 0;
                // Reset NVM
                if( NvmDataMgmtFactoryReset( ) == true )
                {
                    am_util_stdio_printf( "\n\nNVM factory reset succeed\n" );
                }
                else
                {
                    am_util_stdio_printf( "\n\nNVM factory reset failed\n" );
                }
                
                am_util_stdio_printf( "\n\nPLEASE RESET THE END-DEVICE\n\n" );
                while( 1 );
            }
        }
    }
}