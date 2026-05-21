/*!
 * \file      rtc-board.c
 *
 * \brief     Target board RTC timer and low power modes management
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
 *
 * \author 	  Timm Luhmann (Semtech)
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "timer.h"
#include "device.h"


/*!
 * \brief Correction factors
 */
#define  DAYS_IN_MONTH_CORRECTION_NORM              ( ( uint32_t )0x99AAA0 )
#define  DAYS_IN_MONTH_CORRECTION_LEAP              ( ( uint32_t )0x445550 )

/*!
 * \brief Calculates ceiling( X / N )
 */
#define DIVC( X, N )                                ( ( ( X ) + ( N ) -1 ) / ( N ) )

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

#define MIN_ALARM_DELAY 							1

typedef struct
{
	uint32_t			Time;
	am_hal_rtc_time_t 	CalenderTime;
}RtcTimerContext_t;

static bool RtcInitialized = false;

static RtcTimerContext_t RtcTimerContext;
static uint32_t rtcBackup[2];

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * \brief Get the current time from calendar in ticks
 *
 * \param [IN] currentTime           Pointer to RTC_TimeStruct
 * \retval calendarValue Time in ticks
 */
uint32_t RtcGetCalendarValue(am_hal_rtc_time_t* currentTime);

uint32_t RtcGetTimerValue( void );

void RtcStartAlarm( uint32_t timeout );

/*!
 * \brief Initializes the RTC timer
 *
 * \remark The timer is based on the RTC
 */
void RtcInit( void ){
	if(RtcInitialized == false){
		am_hal_rtc_time_t currentTime;
		currentTime.ui32Year = 23;
		currentTime.ui32Month = 1;
		currentTime.ui32DayOfMonth = 1;
		currentTime.ui32Hour = 0;
		currentTime.ui32Minute = 0;
		currentTime.ui32Second = 0;
		currentTime.ui32Hundredths = 0;
		currentTime.ui32Century = 0;
		currentTime.ui32Weekday = 0;
		
		am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_START, 0);
		am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);		//select external OSC
		am_hal_rtc_osc_enable();
		am_hal_rtc_time_12hour(false);					//select 24 hour format
		am_hal_rtc_time_set(&currentTime);
	}
	
}

/*!
 * \brief Sets the RTC timer reference
 *
 * \retval value Timer reference value in ticks
 */
uint32_t RtcSetTimerContext( void ){
	RtcTimerContext.Time = RtcGetCalendarValue(&RtcTimerContext.CalenderTime);
	return RtcTimerContext.Time;
}

/*!
 * \brief Gets the RTC timer reference
 *
 * \retval value Timer value in ticks
 */
uint32_t RtcGetTimerContext( void ){
	return RtcTimerContext.Time;
}

/*!
 * \brief Returns the minimum timeout value
 *
 * \retval minTimeout Minimum timeout value in in ticks
 */
uint32_t RtcGetMinimumTimeout( void ){
	return MIN_ALARM_DELAY;
}

/*!
 * \brief converts time in ms to time in ticks
 *
 * \param[IN] milliseconds Time in milliseconds
 * \retval returns time in timer ticks
 */
uint32_t RtcMs2Tick( TimerTime_t milliseconds ){
	return milliseconds / 10;
}

/*!
 * \brief converts time in ticks to time in ms
 *
 * \param[IN] time in timer ticks
 * \retval returns time in milliseconds
 */
TimerTime_t RtcTick2Ms( uint32_t tick ){
	return tick*10;
}

/*!
 * \brief Performs a delay of milliseconds by polling RTC
 *
 * \param[IN] milliseconds Delay in ms
 */
void RtcDelayMs( TimerTime_t milliseconds ){
	uint32_t delayTicks = 0;
    uint32_t refTicks = RtcGetTimerValue( );
	delayTicks = RtcMs2Tick(milliseconds);
	//wait delay ticks
	while( (RtcGetTimerValue() - refTicks) < delayTicks){
		__NOP();
	}
}

/*!
 * \brief Sets the alarm
 *
 * \note The alarm is set at now (read in this funtion) + timeout
 *
 * \param timeout [IN] Duration of the Timer ticks
 */
void RtcSetAlarm( uint32_t timeout ){
	
	RtcStartAlarm(timeout);		//currently this function has no real application: 
}

/*!
 * \brief Stops the Alarm
 */
void RtcStopAlarm( void ){
	am_hal_rtc_int_disable(AM_HAL_RTC_INT_ALM);	//disable RTC alarm interrupt
	am_hal_rtc_int_clear(AM_HAL_RTC_INT_ALM);	//clear RTC alarm interrupt flag
}

/*!
 * \brief Starts wake up alarm
 *
 * \note  Alarm in RtcTimerContext.Time + timeout
 *
 * \param [IN] timeout Timeout value in ticks
 */
void RtcStartAlarm( uint32_t timeout ){
	uint16_t rtcAlarmSubSeconds = 0;
    uint16_t rtcAlarmSeconds = 0;
    uint16_t rtcAlarmMinutes = 0;
    uint16_t rtcAlarmHours = 0;
    uint16_t rtcAlarmDays = 0;
	am_hal_rtc_time_t alarmTime;
	
	RtcSetTimerContext();		//update the timercontext
	
	RtcStopAlarm();
	rtcAlarmSubSeconds = (timeout%100);	// rest of the convertion to seconds equals the 100ths
	timeout = (timeout-rtcAlarmSubSeconds)/100;	//convert into seconds, minus the 100ths seconds
	
	//convert timeout ticks into 100th, seconds, minutes, hours and days
	rtcAlarmDays = RtcTimerContext.CalenderTime.ui32DayOfMonth;
	while( timeout >= SECONDS_IN_1DAY )
    {
        timeout -= SECONDS_IN_1DAY;
        rtcAlarmDays++;
    }
	rtcAlarmHours = RtcTimerContext.CalenderTime.ui32Hour;
	while( timeout >= SECONDS_IN_1HOUR )
    {
        timeout -= SECONDS_IN_1HOUR;
        rtcAlarmHours++;
    }
	rtcAlarmMinutes = RtcTimerContext.CalenderTime.ui32Minute;
	while( timeout >= SECONDS_IN_1MINUTE )
    {
        timeout -= SECONDS_IN_1MINUTE;
        rtcAlarmMinutes++;
    }
	rtcAlarmSeconds = RtcTimerContext.CalenderTime.ui32Second + timeout;
	
	//Correction for overflows
	while( rtcAlarmSeconds >= SECONDS_IN_1MINUTE )
    { 
        rtcAlarmSeconds -= SECONDS_IN_1MINUTE;
        rtcAlarmMinutes++;
    }

    while( rtcAlarmMinutes >= MINUTES_IN_1HOUR )
    {
        rtcAlarmMinutes -= MINUTES_IN_1HOUR;
        rtcAlarmHours++;
    }

    while( rtcAlarmHours >= HOURS_IN_1DAY )
    {
        rtcAlarmHours -= HOURS_IN_1DAY;
        rtcAlarmDays++;
    }

    if( RtcTimerContext.CalenderTime.ui32Year % 4 == 0 ) 
    {
        if( rtcAlarmDays > DaysInMonthLeapYear[RtcTimerContext.CalenderTime.ui32Month - 1] )			//-1 for shifted array representation
        {
            rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[RtcTimerContext.CalenderTime.ui32Month - 1];
        }
    }
    else
    {
        if( rtcAlarmDays > DaysInMonth[RtcTimerContext.CalenderTime.ui32Month - 1] )
        {   
            rtcAlarmDays = rtcAlarmDays % DaysInMonth[RtcTimerContext.CalenderTime.ui32Month - 1];
        }
    }
	alarmTime.ui32Hundredths = rtcAlarmSubSeconds;
	alarmTime.ui32Second = rtcAlarmSeconds;
	alarmTime.ui32Minute = rtcAlarmMinutes;
	alarmTime.ui32Hour = rtcAlarmHours;
	alarmTime.ui32DayOfMonth = rtcAlarmDays;
	alarmTime.ui32Century = RtcTimerContext.CalenderTime.ui32Century;
	alarmTime.ui32Weekday = RtcTimerContext.CalenderTime.ui32Weekday;
	alarmTime.ui32Year = RtcTimerContext.CalenderTime.ui32Year;
	alarmTime.ui32Month = RtcTimerContext.CalenderTime.ui32Month;	
	am_hal_rtc_int_enable(AM_HAL_RTC_INT_ALM);
	am_hal_interrupt_master_enable();
	NVIC_EnableIRQ(RTC_IRQn);
	am_hal_rtc_alarm_set(&alarmTime,AM_HAL_RTC_ALM_RPT_YR);
	//am_hal_rtc_int_set(AM_HAL_RTC_INT_ALM);
}

/*!
 * \brief Gets the system reference Time in ticks
 * \param [IN] currentTime 			Pointer to current RTC time
 * \retval timer ticks since epoch
 */
uint32_t RtcGetCalendarValue(am_hal_rtc_time_t* currentTime){
	uint32_t calendarValue = 0;
	uint32_t correction;
    uint32_t seconds;
	am_hal_rtc_time_get(currentTime);
	// Calculte amount of elapsed days (full year) since 01/01/2000
    seconds = DIVC( ( DAYS_IN_YEAR * 3 + DAYS_IN_LEAP_YEAR ) * currentTime->ui32Year , 4 );	//ceiling value for days 
	correction = ( ( currentTime->ui32Year % 4 ) == 0 ) ? DAYS_IN_MONTH_CORRECTION_LEAP : DAYS_IN_MONTH_CORRECTION_NORM;	//select correction factor
	seconds += ( DIVC( ( currentTime->ui32Month-1 ) * ( 30 + 31 ), 2 ) - ( ( ( correction >> ( ( currentTime->ui32Month - 1 ) * 2 ) ) & 0x03 ) ) );
	seconds += ( currentTime->ui32DayOfMonth -1 );
	// Convert from days to seconds
    seconds *= SECONDS_IN_1DAY;
	seconds += currentTime->ui32Hour * SECONDS_IN_1HOUR;	//add hours of current day
	seconds += currentTime->ui32Minute * SECONDS_IN_1MINUTE;	//add minutes of current hour
	seconds += currentTime->ui32Second;						//add current seconds
	
	calendarValue = seconds*100;	//convert the seconds to timerticks(apollo rtc uses 100 Hz clock)
	return calendarValue;
}


/*!
 * \brief Gets the system time with the number of seconds elapsed since epoch
 *
 * \param [OUT] milliseconds Number of milliseconds elapsed since epoch
 * \retval seconds Number of seconds elapsed since epoch
 */
uint32_t RtcGetCalendarTime( uint16_t *milliseconds ){
	uint32_t ticks = 0;
	ticks = RtcGetCalendarValue(&RtcTimerContext.CalenderTime);
	*milliseconds = ticks * 10;
	return ticks/100;
}

/*!
 * \brief Get the RTC timer value
 *
 * \retval RTC Timer value
 */
uint32_t RtcGetTimerValue( void ){
	return RtcGetCalendarValue(&RtcTimerContext.CalenderTime);
}

/*!
 * \brief Get the RTC timer elapsed time since the last Alarm was set
 *
 * \retval RTC Elapsed time since the last alarm in ticks.
 */
uint32_t RtcGetTimerElapsedTime( void ){
	uint32_t calendarValue = ( uint32_t )RtcGetCalendarValue(&RtcTimerContext.CalenderTime);
	return calendarValue - RtcTimerContext.Time;
}




void am_rtc_isr(void){
	//gpioWrite(ADC_SENSOREXTRA2,false);
	if(am_hal_rtc_int_status_get(true)){
		am_hal_rtc_int_clear(AM_HAL_RTC_INT_ALM);
		TimerIrqHandler();
	}
	
	
}


/*!
 * \brief Writes data0 and data1 to the RTC backup registers
 *
 * \param [IN] data0 1st Data to be written
 * \param [IN] data1 2nd Data to be written
 * This is not optimal since the data will be lost on powerdown
 */
void RtcBkupWrite( uint32_t data0, uint32_t data1 ){
	rtcBackup[0] = data0;
	rtcBackup[1] = data1;
}

/*!
 * \brief Reads data0 and data1 from the RTC backup registers
 *
 * \param [OUT] data0 1st Data to be read
 * \param [OUT] data1 2nd Data to be read
 */
void RtcBkupRead( uint32_t* data0, uint32_t* data1 ){
	*data0 = rtcBackup[0];
	*data1 = rtcBackup[1];
}

/*!
 * \brief Processes pending timer events
 */
void RtcProcess( void ){
	//not used on this platform
}

/*!
 * \brief Computes the temperature compensation for a period of time on a
 *        specific temperature.
 *
 * \param [IN] period Time period to compensate in milliseconds
 * \param [IN] temperature Current temperature
 *
 * \retval Compensated time period
 */
TimerTime_t RtcTempCompensation( TimerTime_t period, float temperature ){
	// The drift equation of a tuning fork crystal over temperature is
    //
    //   delta_f / f_0 = k * (T - T_0) ^ 2
    //
    // As the crystal is external to the module, typical values range
    // from 0.03 to 0.04.  For the ASB06-32768, k is -0.03 nominal and T_0 is
    // at 25C nominal.
    float T_0 = 25.0f;
    float k   = -0.034f;
    float dev = 0.0f;

    dev = k * (temperature - T_0) * (temperature - T_0);

    // for component accuracy in the ppb range, change
    // the divisor to 1.0e9f
    float correction = ((float)period * dev) / 1.0e6f;

    // This would never happen practically, but for the sake of
    // mathematical correctness we should use a signed integer to
    // accommodate negative value correction
    int32_t corrected_period = (int32_t)period + round(correction);

    // re-cast the value back to uint32_t (TimerTime_t)
    return (TimerTime_t)corrected_period;
}

#ifdef __cplusplus
}
#endif

