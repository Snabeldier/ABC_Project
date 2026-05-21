/*!
 *  \file       am1805.h
 *
 *  \brief      Provides functions to use the external AM1805 RTC
 *
 *  \date       03.02.2023
 *
 *  \author     Timm Luhmann (IMTEK)
 */
#ifndef AM1805_H
#define AM1805_H


#include "stdint.h"
//*****************************************************************************
//
// Global definitions for the commands
//
//*****************************************************************************
#define AM_DEVICES_AM1805_SLAVE_ID          0x69

#define AM_DEVICES_AM1805_MAX_DEVICE_NUM    1

#define ADDRESS_HUNDREDTHS 0x00
#define ADDRESS_SECONDS 0x01
#define ADDRESS_MINUTES 0x02
#define ADDRESS_HOURS 0x03
#define ADDRESS_dayOfMonth 0x04
#define ADDRESS_MONTHS 0x05
#define ADDRESS_YEARS 0x06
#define ADDRESS_WEEKDAYS 0x07
#define ADDRESS_ALARM_HUNDREDTHS 0x08
#define ADDRESS_ALARM_SECONDS 0x09
#define ADDRESS_ALARM_MINUTES 0x0A
#define ADDRESS_ALARM_HOURS 0x0B
#define ADDRESS_ALARM_dayOfMonth 0x0C
#define ADDRESS_ALARM_MONTHS 0x0D
#define ADDRESS_ALARM_WEEKDAYS 0x0E
#define ADDRESS_STATUS 0x0F
#define ADDRESS_CONTROL1 0x10
#define ADDRESS_CONTROL2 0x11
#define ADDRESS_INTMASK 0x12
#define ADDRESS_SQW 0x13
#define ADDRESS_CAL_XT 0x14
#define ADDRESS_CAL_RC_HI 0x15
#define ADDRESS_CAL_RC_LOW 0x16
#define ADDRESS_SLEEP_CONTROL 0x17
#define ADDRESS_TIMER_CONTROL 0x18
#define ADDRESS_TIMER 0x19
#define ADDRESS_TIMER_INIT 0x1A
#define ADDRESS_WDT 0x1B
#define ADDRESS_OSC_CONTROL 0x1C
#define ADDRESS_OSC_STATUS 0x1D
#define ADDRESS_CONFIG_KEY 0x1F

//control 1 register
#define OPERATION24 0x40
#define POWERSWITCH 0x02
#define WRITE_RTC 0x01
#define AUTORESET 0x04
#define RSP 0x08

//control 2 register
#define PSW_SLEEP 0x18

//interrupt mask register
#define CENTURY_ENABLE 0x80
#define INTERRUPT_MODE_RESET 0x60
#define BATTERY_LOW_ENABLE 0x10
#define TIMER_INTERRUPT_ENABLE 0x08
#define ALARM_INTERRUPT_ENABLE 0x04
#define EXT2_INTERRUPT_ENABLE 0x02
#define EXT1_INTERRUPT_ENABLE 0x01

//sleep control register
#define SLP 0x80
#define SLRES 0x40
#define EX2P 0x20
#define EX1P 0x10
#define SLST 0x08
#define SLP_DELAY_0 0x00
#define SLP_DELAY_MAX 0x07

//timer control register
#define TE 0x80
#define TM 0x40
#define TRPT 0x20
#define RPT_ONCE_PER_SECOND 0x1C
#define RPT_ONCE_PER_MINUTE 0x18
#define RPT_ONCE_PER_HOUR 0x14
#define RPT_ONCE_PER_DAY 0x10
#define RPT_ONCE_PER_WEEK 0x0C
#define RPT_ONCE_PER_MONTH 0x08
#define RPT_ONCE_PER_YEAR 0x04
#define RPT_ALARM_DISABLE 0x00

//oscillator control bits
#define PWGT 0x04

//configuration key
#define CFG_KEY 0xA1


typedef enum
{
    AM_DEVICES_AM1805_STATUS_SUCCESS,
    AM_DEVICES_AM1805_STATUS_ERROR
} am_devices_am1805_status_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} am_devices_am1805_config_t;

typedef struct
{
	uint8_t hundredth;
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t dayOfMonth;
	uint8_t weekday;
	uint8_t month;
	uint8_t year;
	uint8_t mode;
} time_struct;

uint32_t am_devices_am1805_read_id(uint32_t *pDeviceID);
uint32_t am_devices_am1805_config_init();
uint32_t am_devices_am1805_get_time(time_struct *time_current);
uint32_t am_devices_am1805_set_time(time_struct *time_current);
uint32_t am_devices_am1805_set_alarmTime(time_struct *alarm_time);
uint32_t am_devices_am1805_config_sleep(uint8_t sleepDays, uint8_t sleepHours, uint8_t sleepMinutes, uint8_t sleepSeconds);
uint32_t am_devices_am1805_read_status(uint32_t *pStatus);
uint32_t am_devices_am1805_read_register(uint32_t *pStatus, uint32_t reg);
#endif