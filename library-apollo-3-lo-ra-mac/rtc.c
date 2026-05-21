/***************************************************************************//**
 *  \file       rtc.c
 *
 *  \brief      Provides functions to control the internal RTC
 *
 *  \date       06.03.2024
 *
 *  \author     Timm Luhmann ( Uni Freiburg IMTEK )
 ******************************************************************************/
 
 #include "rtc.h"
 
 /******************************************************************************
 * VARIABLES
 *****************************************************************************/
 static bool RtcInitialized = false;
static am_hal_rtc_time_t 	timerContext;

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

void rtcGetTimerValue(void);

void rtcGetTimerValue(void){
	am_hal_rtc_time_get(&timerContext);
}


uint32_t rtcInit( void ){
	uint32_t error = 0;
	if(RtcInitialized == false){
		am_hal_rtc_time_t currentTime;
		currentTime.ui32Year = CURRENTYEAR;
		currentTime.ui32Month = CURRENTMONTH;
		currentTime.ui32DayOfMonth = CURRENTDAYOFMONTH;
		currentTime.ui32Hour = CURRENTHOUR;
		currentTime.ui32Minute = CURRENTMINUTE;
		currentTime.ui32Second = CURRENTSECOND;
		currentTime.ui32Hundredths = 0;
		currentTime.ui32Century = 0;
		currentTime.ui32Weekday = 0;
		
		error += am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_START, 0);
		am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);		//select external OSC
		am_hal_rtc_osc_enable();
		am_hal_rtc_time_12hour(false);					//select 24 hour format
		am_hal_rtc_time_set(&currentTime);
	}
	return error;
}

void rtcSetTime(am_hal_rtc_time_t *setTime){
	
	am_hal_rtc_time_set(setTime);
}


uint32_t rtcReadTime(am_hal_rtc_time_t *readTime){
	
	return am_hal_rtc_time_get(readTime);
}

//timeout given in seconds
uint32_t rtcSetAlarm(uint32_t timeout){
	uint32_t error = 0;
	uint16_t rtcAlarmSeconds = 0;
    uint16_t rtcAlarmMinutes = 0;
    uint16_t rtcAlarmHours = 0;
    uint16_t rtcAlarmDays = 0;
	am_hal_rtc_time_t alarmTime;
	
	rtcStopAlarm();
	rtcGetTimerValue();
	
	rtcAlarmDays = timerContext.ui32DayOfMonth;		//get current day of month
	while( timeout >= SECONDS_IN_1DAY )
    {
        timeout -= SECONDS_IN_1DAY;
        rtcAlarmDays++;
    }
	rtcAlarmHours = timerContext.ui32Hour;			//get current hour
	while( timeout >= SECONDS_IN_1HOUR )
    {
        timeout -= SECONDS_IN_1HOUR;
        rtcAlarmHours++;
    }
	rtcAlarmMinutes = timerContext.ui32Minute;		//get current minute
	while( timeout >= SECONDS_IN_1MINUTE )
    {
        timeout -= SECONDS_IN_1MINUTE;
        rtcAlarmMinutes++;
    }
	rtcAlarmSeconds = timerContext.ui32Second + timeout;
	
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

    if( timerContext.ui32Year % 4 == 0 ) 
    {
        if( rtcAlarmDays > DaysInMonthLeapYear[timerContext.ui32Month - 1] )			//-1 for shifted array representation
        {
            rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[timerContext.ui32Month - 1];
        }
    }
    else
    {
        if( rtcAlarmDays > DaysInMonth[timerContext.ui32Month - 1] )
        {   
            rtcAlarmDays = rtcAlarmDays % DaysInMonth[timerContext.ui32Month - 1];
        }
    }
	
	alarmTime.ui32Hundredths = 0;
	alarmTime.ui32Second = rtcAlarmSeconds;
	alarmTime.ui32Minute = rtcAlarmMinutes;
	alarmTime.ui32Hour = rtcAlarmHours;
	alarmTime.ui32DayOfMonth = rtcAlarmDays;
	alarmTime.ui32Century = timerContext.ui32Century;
	alarmTime.ui32Weekday = timerContext.ui32Weekday;
	alarmTime.ui32Year = timerContext.ui32Year;
	alarmTime.ui32Month = timerContext.ui32Month;	
	am_hal_rtc_int_enable(AM_HAL_RTC_INT_ALM);
	error += am_hal_interrupt_master_enable();
	NVIC_EnableIRQ(RTC_IRQn);
	am_hal_rtc_alarm_set(&alarmTime,AM_HAL_RTC_ALM_RPT_YR);	
	return error;
}


void rtcSleep(uint32_t seconds){
	rtcSetAlarm(seconds);
	am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_NORMALSLEEP);
}

/*!
 * \brief Stops the Alarm
 */
void rtcStopAlarm( void ){
	am_hal_rtc_int_disable(AM_HAL_RTC_INT_ALM);	//disable RTC alarm interrupt
	am_hal_rtc_int_clear(AM_HAL_RTC_INT_ALM);	//clear RTC alarm interrupt flag
}

void am_rtc_isr(void){
	//gpioWrite(ADC_SENSOREXTRA2,false);
	if(am_hal_rtc_int_status_get(true)){
		am_hal_rtc_int_clear(AM_HAL_RTC_INT_ALM);
	}
}