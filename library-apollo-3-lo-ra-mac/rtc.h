/*!
 *  \file       rtc.h
 *
 *  \brief      Provides functions to configure the rtc
 *
 *  \date       06.03.2024
 *
 *  \author     Uttunga Shinde (IMTEK)
 */
#ifndef RTC_H
#define RTC_H

#include "am_mcu_apollo.h"

#define CURRENTYEAR 24
#define CURRENTMONTH 3
#define CURRENTDAYOFMONTH 4
#define CURRENTHOUR 12
#define CURRENTMINUTE 43
#define CURRENTSECOND 12

/*!
 * \brief Days, Hours, Minutes and seconds
 */
#define DAYS_IN_LEAP_YEAR                           ( ( uint32_t )  366U )
#define DAYS_IN_YEAR                                ( ( uint32_t )  365U )
#define SECONDS_IN_1DAY                             ( ( uint32_t )86400U )
#define SECONDS_IN_1HOUR                            ( ( uint32_t ) 3600U )
#define SECONDS_IN_1MINUTE                          ( ( uint32_t )   60U )
#define MINUTES_IN_1HOUR                            ( ( uint32_t )   60U )
#define HOURS_IN_1DAY                               ( ( uint32_t )   24U )

#define TM_SECONDS_IN_1DAY							84640000U	//seconds in 1 day in ticks (*100)
#define TM_SECONDS_IN_1HOUR							360000U		//seconds in 1 hour in ticks (*100)
#define TM_SECONDS_IN_1MINUTE						6000U		//seconds in 1 minute in ticks (*100)


/*
 *	@brief Initializes the RTC for sleep & timekeeping operation
 *
 *	@return 0 on success
 */
uint32_t rtcInit(void);

/*
 *	@brief Sets the time in the RTC
 */
void rtcSetTime(am_hal_rtc_time_t *setTime);

/*
 *	@brief Reads the current time in the RTC
 *
 *	@return 0 on success
 */
uint32_t rtcReadTime(am_hal_rtc_time_t *readTime);

/*
 *	@brief Set the alarm on the RTC for waking up
 *
 *	@return 0 on success
 */
uint32_t rtcSetAlarm(uint32_t seconds);	

/*
 *	@brief Set the controller into sleep & wait till given amount of seconds passed. Then wake up
 */
void rtcSleep(uint32_t seconds);

/*
 *	@brief Stop any pending alarm
 */
void rtcStopAlarm(void);

#endif