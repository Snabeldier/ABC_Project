/*!
 *  \file       adc.c
 *
 *  \brief      Provides functions to configure the adc
 *
 *  \date       01.03.2023
 *
 *  \author     Timm Luhmann (IMTEK)
 */
//***** Header Files **********************************************************
//#include <stdint.h>
#include <stdbool.h>
#include "math.h"
#include "am_mcu_apollo.h"
#include "pins.h"
#include "adc.h"
#include "am_util_stdio.h"
#include "boardControl.h"


//***** Defines ***************************************************************

#define TIMER3CONTROLLED
//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//
// ADC Device Handle.
//
static void *g_ADCHandle;

uint32_t adcValue = 0;

bool conversionFinished;

//
// ADC Sample buffer.
//
#define ADC_SAMPLE_BUF_SIZE 512
uint32_t g_ui32ADCSampleBuffer[ADC_SAMPLE_BUF_SIZE] = { 0 };

//
// ADC DMA complete flag.
//
volatile bool                   g_bADCDMAComplete;

//
// ADC DMA error flag.
//
volatile bool                   g_bADCDMAError;


//*****************************************************************************
//
// ADC Configuration
//
//*****************************************************************************

const static am_hal_adc_config_t g_sADC_Cfg_Cap =
{
    //
    // Select the ADC Clock source.
    //
    .eClock = AM_HAL_ADC_CLKSEL_HFRC_DIV2,

    //
    // Polarity
    //
    .ePolarity = AM_HAL_ADC_TRIGPOL_RISING,

    //
    // Select the ADC trigger source using a trigger source macro.
    //
    .eTrigger = AM_HAL_ADC_TRIGSEL_SOFTWARE,

    //
    // Select the ADC reference voltage.
    //
    .eReference = AM_HAL_ADC_REFSEL_INT_1P5,
    .eClockMode = AM_HAL_ADC_CLKMODE_LOW_POWER,

    //
    // Choose the power mode for the ADC's idle state.
    //
    .ePowerMode = AM_HAL_ADC_LPMODE1,

    //
    // Enable repeating samples using Timer3A.
    //
    .eRepeat = AM_HAL_ADC_SINGLE_SCAN
};


const static am_hal_adc_config_t g_sADC_Cfg_PAM =
{
    //
    // Select the ADC Clock source.
    //
    .eClock = AM_HAL_ADC_CLKSEL_HFRC,

    //
    // Polarity
    //
    .ePolarity = AM_HAL_ADC_TRIGPOL_RISING,

    //
    // Select the ADC trigger source using a trigger source macro.
    //
    .eTrigger = AM_HAL_ADC_TRIGSEL_SOFTWARE,

    //
    // Select the ADC reference voltage.
    //
    .eReference = AM_HAL_ADC_REFSEL_INT_1P5,
    .eClockMode = AM_HAL_ADC_CLKMODE_LOW_POWER,

    //
    // Choose the power mode for the ADC's idle state.
    //
	#ifdef PAM_OPTIMIZED
	.ePowerMode = AM_HAL_ADC_LPMODE1,
	#else
    .ePowerMode = AM_HAL_ADC_LPMODE1,
	#endif
    //
    // Enable repeating samples using Timer3A.
    //
    .eRepeat = AM_HAL_ADC_REPEATING_SCAN
};

//*****************************************************************************
//
// Timer configurations.
//
//*****************************************************************************
am_hal_ctimer_config_t g_sTimer3 =
{
    // do not link A and B together to make a long 32-bit counter.
    0,

    // Set up timer 3A to drive the ADC
    (AM_HAL_CTIMER_FN_PWM_REPEAT |
     AM_HAL_CTIMER_HFRC_3MHZ),

    // Timer 3B is not used in this example.
    0,
};


//*****************************************************************************
//
// ADC INIT Function for the ADC operation to measure the voltage on the Capacitor
//
//*****************************************************************************
void
adc_init_Cap(void)
{
    am_hal_adc_slot_config_t sSlotCfg;
	
	uint32_t error = 0;

    //
    // Initialize the ADC and get the handle.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_adc_initialize(0, &g_ADCHandle) )
    {
        error += 1;
    }

    //
    // Power on the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
                                                          AM_HAL_SYSCTRL_WAKE,
                                                          false) )
    {
        error += 1;
    }

    //
    // Configure the ADC.
    //
    if ( am_hal_adc_configure(g_ADCHandle, (am_hal_adc_config_t*)&g_sADC_Cfg_Cap) != AM_HAL_STATUS_SUCCESS )
    {
        error += 1;
    }

    sSlotCfg.bEnabled       = false;
    sSlotCfg.bWindowCompare = false;
    sSlotCfg.eChannel       = AM_HAL_ADC_SLOT_CHSEL_SE0;    // 0
    sSlotCfg.eMeasToAvg     = AM_HAL_ADC_SLOT_AVG_1;        // 0
    sSlotCfg.ePrecisionMode = AM_HAL_ADC_SLOT_14BIT;        // 0

    am_hal_adc_configure_slot(g_ADCHandle, 0, &sSlotCfg);   // Unused slot
    am_hal_adc_configure_slot(g_ADCHandle, 1, &sSlotCfg);   // Unused slot
    am_hal_adc_configure_slot(g_ADCHandle, 2, &sSlotCfg);   // Unused slot
    am_hal_adc_configure_slot(g_ADCHandle, 3, &sSlotCfg);   // Unused slot
    am_hal_adc_configure_slot(g_ADCHandle, 4, &sSlotCfg);   // Unused slot
	am_hal_adc_configure_slot(g_ADCHandle, 6, &sSlotCfg);   // Unused slot
	am_hal_adc_configure_slot(g_ADCHandle, 5, &sSlotCfg);   // Unused slot

    sSlotCfg.bEnabled       = true;
    sSlotCfg.bWindowCompare = true;
    //sSlotCfg.eChannel       = AM_HAL_ADC_SLOT_CHSEL_SE3;	//Pin D31 - Capacitor from Powerboard	
	sSlotCfg.eChannel       = AM_HAL_ADC_SLOT_CHSEL_TEMP;
	#ifdef AVERAGE_ADC
    sSlotCfg.eMeasToAvg     = AM_HAL_ADC_SLOT_AVG_1;		//use averaging of 8
	#else
	sSlotCfg.eMeasToAvg		= AM_HAL_ADC_SLOT_AVG_1;
	#endif
    sSlotCfg.ePrecisionMode = AM_HAL_ADC_SLOT_10BIT;
    am_hal_adc_configure_slot(g_ADCHandle, 7, &sSlotCfg);   // Capacitor Powerboard

//    sSlotCfg.bEnabled       = true;
//    sSlotCfg.bWindowCompare = true;
//    sSlotCfg.eChannel       = AM_HAL_ADC_SLOT_CHSEL_SE9;	//Pin D12 - OPV from Sensorboard	
//    sSlotCfg.eMeasToAvg     = AM_HAL_ADC_SLOT_AVG_1;
//    sSlotCfg.ePrecisionMode = AM_HAL_ADC_SLOT_12BIT;
//    am_hal_adc_configure_slot(g_ADCHandle, 5, &sSlotCfg);   // PAMPE OPV

//    sSlotCfg.bEnabled       = true;
//    sSlotCfg.bWindowCompare = true;
//    sSlotCfg.eChannel       = AM_HAL_ADC_SLOT_CHSEL_SE8;	//Pin D13 - Bias from Sensorboard	
//    sSlotCfg.eMeasToAvg     = AM_HAL_ADC_SLOT_AVG_1;
//    sSlotCfg.ePrecisionMode = AM_HAL_ADC_SLOT_14BIT;
//    am_hal_adc_configure_slot(g_ADCHandle, 7, &sSlotCfg);   // PAMPE Bias
	error += am_hal_gpio_pinconfig(ADC_PAM,pad0FNCsel);                      //Pam Pin set to ADC mode
	error += am_hal_gpio_pinconfig(ADC_BIAS,pad0FNCsel);    				  //Vdd/2 signal from Pam measurement
    error += am_hal_gpio_pinconfig(ADC_CAPACITOR,pad0FNCsel);    				  //Vdd/2 signal from Pam measurement
	
	//
	// For this example, the samples will be coming in slowly. This means we
	// can afford to wake up for every conversion.
	//
	am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_CNVCMP );		//conversion complete interrupt enabled
//	am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_WCINC       |
//										 AM_HAL_ADC_INT_WCEXC       |
//										 AM_HAL_ADC_INT_FIFOOVR2    |
//										 AM_HAL_ADC_INT_FIFOOVR1    |
//										 AM_HAL_ADC_INT_SCNCMP      |
//										 AM_HAL_ADC_INT_CNVCMP);		//conversion complete interrupt enabled

    //
    // Enable the ADC.
    //
    am_hal_adc_enable(g_ADCHandle);
	
}
//*****************************************************************************
//
// ADC INIT Function for the ADC operation to measure the PAM
//
//*****************************************************************************
void
adc_init_PAM(void * bufferAddress)
{
    am_hal_adc_slot_config_t sSlotCfg;
	
	uint32_t error = 0;

    //
    // Initialize the ADC and get the handle.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_adc_initialize(0, &g_ADCHandle) )
    {
        error += 1;
    }

    //
    // Power on the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
                                                          AM_HAL_SYSCTRL_WAKE,
                                                          false) )
    {
        error += 1;
    }

    //
    // Configure the ADC.
    //
    if ( am_hal_adc_configure(g_ADCHandle, (am_hal_adc_config_t*)&g_sADC_Cfg_PAM) != AM_HAL_STATUS_SUCCESS )
    {
        error += 1;
    }

    sSlotCfg.bEnabled       = false;
    sSlotCfg.bWindowCompare = false;
    sSlotCfg.eChannel       = AM_HAL_ADC_SLOT_CHSEL_SE0;    // 0
    sSlotCfg.eMeasToAvg     = AM_HAL_ADC_SLOT_AVG_1;        // 0
    sSlotCfg.ePrecisionMode = AM_HAL_ADC_SLOT_14BIT;        // 0

    am_hal_adc_configure_slot(g_ADCHandle, 0, &sSlotCfg);   // Unused slot
    am_hal_adc_configure_slot(g_ADCHandle, 1, &sSlotCfg);   // Unused slot
    am_hal_adc_configure_slot(g_ADCHandle, 2, &sSlotCfg);   // Unused slot
    am_hal_adc_configure_slot(g_ADCHandle, 3, &sSlotCfg);   // Unused slot
    am_hal_adc_configure_slot(g_ADCHandle, 4, &sSlotCfg);   // Unused slot
	am_hal_adc_configure_slot(g_ADCHandle, 6, &sSlotCfg);   // Unused slot
	am_hal_adc_configure_slot(g_ADCHandle, 7, &sSlotCfg);   // Unused slot

//    sSlotCfg.bEnabled       = true;
//    sSlotCfg.bWindowCompare = true;
//    sSlotCfg.eChannel       = AM_HAL_ADC_SLOT_CHSEL_SE3;	//Pin D31 - Capacitor from Powerboard	
//    sSlotCfg.eMeasToAvg     = AM_HAL_ADC_SLOT_AVG_1;
//    sSlotCfg.ePrecisionMode = AM_HAL_ADC_SLOT_12BIT;
//    am_hal_adc_configure_slot(g_ADCHandle, 6, &sSlotCfg);   // Capacitor Powerboard

    sSlotCfg.bEnabled       = true;
    sSlotCfg.bWindowCompare = true;
    sSlotCfg.eChannel       = AM_HAL_ADC_SLOT_CHSEL_SE9;	//Pin D12 - OPV from Sensorboard	
    sSlotCfg.eMeasToAvg     = AM_HAL_ADC_SLOT_AVG_1;
    sSlotCfg.ePrecisionMode = AM_HAL_ADC_SLOT_12BIT;
    am_hal_adc_configure_slot(g_ADCHandle, 5, &sSlotCfg);   // PAMPE OPV

//    sSlotCfg.bEnabled       = true;
//    sSlotCfg.bWindowCompare = true;
//    sSlotCfg.eChannel       = AM_HAL_ADC_SLOT_CHSEL_SE8;	//Pin D13 - Bias from Sensorboard	
//    sSlotCfg.eMeasToAvg     = AM_HAL_ADC_SLOT_AVG_1;
//    sSlotCfg.ePrecisionMode = AM_HAL_ADC_SLOT_14BIT;
//    am_hal_adc_configure_slot(g_ADCHandle, 7, &sSlotCfg);   // PAMPE Bias
	error += am_hal_gpio_pinconfig(ADC_PAM,pad0FNCsel);                      //Pam Pin set to ADC mode
	//error += am_hal_gpio_pinconfig(ADC_BIAS,pad0FNCsel);    				  //Vdd/2 signal from Pam measurement
    //error += am_hal_gpio_pinconfig(ADC_CAPACITOR,pad0FNCsel);    				  //Vdd/2 signal from Pam measurement
	#ifdef PAM_DMA
	//
    // Configure the ADC to use DMA for the sample transfer.
    //
    adc_config_dma(bufferAddress);
	
	am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_DERR | AM_HAL_ADC_INT_DCMP );
	//
	// For this example, the samples will be coming in slowly. This means we
	// can afford to wake up for every conversion.
	//
	//am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_CNVCMP );		//conversion complete interrupt enabled
#else 
	am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_WCINC       |
										 AM_HAL_ADC_INT_WCEXC       |
										 AM_HAL_ADC_INT_FIFOOVR2    |
										 AM_HAL_ADC_INT_FIFOOVR1    |
										 AM_HAL_ADC_INT_SCNCMP      |
										 AM_HAL_ADC_INT_CNVCMP);
#endif

    //
    // Enable the ADC.
    //
    am_hal_adc_enable(g_ADCHandle);
	
	timer3a_init();
}

//*****************************************************************************
//
// De-Initialize the ADC
//
//*****************************************************************************
void
adc_deconfig(void)
{
	uint32_t error = 0;
	//
	// Disable the ADC.
	//
	if (AM_HAL_STATUS_SUCCESS != am_hal_adc_disable(g_ADCHandle))
	{
	error += 1;
	}

	//
	// Enable the ADC power domain.
	//
	if (AM_HAL_STATUS_SUCCESS != am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_ADC))
	{
	error += 1;
	}

	//
	// Initialize the ADC and get the handle.
	//
	if (AM_HAL_STATUS_SUCCESS != am_hal_adc_deinitialize(g_ADCHandle))
	{
	error += 1;
	}

}

void
adc_config_dma(void * bufferAddress)
{
    am_hal_adc_dma_config_t       ADCDMAConfig;

    //
    // Configure the ADC to use DMA for the sample transfer.
    //
    ADCDMAConfig.bDynamicPriority = true;
    ADCDMAConfig.ePriority = AM_HAL_ADC_PRIOR_SERVICE_IMMED;
    ADCDMAConfig.bDMAEnable = true;
    ADCDMAConfig.ui32SampleCount = ADC_SAMPLE_BUF_SIZE;
    ADCDMAConfig.ui32TargetAddress = (uint32_t)bufferAddress;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_dma(g_ADCHandle, &ADCDMAConfig))
    {
        am_util_stdio_printf("Error - configuring ADC DMA failed.\n");
    }

    //
    // Reset the ADC DMA flags.
    //
    g_bADCDMAComplete = false;
    g_bADCDMAError = false;
}

//*****************************************************************************
//
// Interrupt handler for the ADC.
//
//*****************************************************************************
void
am_adc_isr(void)
{
	uint32_t            ui32IntMask;
	am_hal_adc_sample_t Sample;
	uint32_t error = 0;
	//turnOn33();

	//
	// Read the interrupt status.
	//
	if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_status(g_ADCHandle, &ui32IntMask, false))
	{
		error += 1;
	}

	//
	// Clear the ADC interrupt.
	//
	if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_clear(g_ADCHandle, ui32IntMask))
	{
		error += 1;
	}
	//
	// If we got a conversion completion interrupt (which should be our only
	// ADC interrupt), go ahead and read the data.
	//
//	if (ui32IntMask & AM_HAL_ADC_INT_CNVCMP)
//	{
//		uint32_t    ui32NumSamples = 1;
//		if (AM_HAL_STATUS_SUCCESS != am_hal_adc_samples_read(g_ADCHandle, false,
//															 NULL,
//															 &ui32NumSamples,
//															 &Sample))
//		{
//		 error += 1;
//		}

//	}
//	am_util_stdio_printf("ADC Slot =  %d\n", Sample.ui32Slot);
//    am_util_stdio_printf("ADC Value = %8.8X\n", Sample.ui32Sample);
	//adcValue = Sample.ui32Sample;
	    //
#ifdef PAM_DMA
    // If we got a DMA complete, set the flag.
    //
    if (ui32IntMask & AM_HAL_ADC_INT_DCMP)
    {
        g_bADCDMAComplete = true;
		adcValue++;
    }

    //
    // If we got a DMA error, set the flag.
    //
    if (ui32IntMask & AM_HAL_ADC_INT_DERR)
    {
        g_bADCDMAError = true;
    }
#endif
	conversionFinished = true;
}

void adcGetValue(uint32_t *adcReturn){
	
	*adcReturn = adcValue;
	
}

void adcTriggerInterrupt(){
	
	am_hal_adc_sw_trigger(g_ADCHandle);
	
}

float getAdcTemperature(){
	adc_init_Cap();
	float temperature;
	uint32_t retVal;
	am_hal_interrupt_master_enable();
	NVIC_EnableIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn,1);	//set lower priority for adc
	am_hal_adc_sample_t Sample;
	conversionFinished = false;
	am_hal_adc_sw_trigger(g_ADCHandle);
	while(!conversionFinished){
		am_util_delay_us(50);
	}
	//ADC->INTSET = 0x01;		//trigger the interrupt for test purpose
	//wait until interrupt is executed
	//while(!conversionFinished);

	uint32_t    ui32NumSamples = 1;
	while ( AM_HAL_ADC_FIFO_COUNT(ADC->FIFO) ){
		if (AM_HAL_STATUS_SUCCESS != am_hal_adc_samples_read(g_ADCHandle, false, NULL, &ui32NumSamples, &Sample)){
			am_util_stdio_printf("Error reading ADC value");
		}		
	}
	temperature = Sample.ui32Sample;
	temperature = temperature * 1.5 / (1024);
	//temperature = temperature * 1.5 / (16384);
	float fVT[3];
	fVT[0] = temperature;
	fVT[1] = 0.0f;
	fVT[2] = -123.456;
//      fADCTempDegreesC = am_hal_adc_volts_to_celsius(fADCTempVolts);
	retVal = am_hal_adc_control(g_ADCHandle, AM_HAL_ADC_REQ_TEMP_CELSIUS_GET, fVT);
	temperature = fVT[1];
	adc_deconfig();
	return temperature;
}

float getAdcSample(){
	adc_init_Cap();
	turnOnVDiv();
	am_util_delay_ms(10);
	am_hal_interrupt_master_enable();
	NVIC_EnableIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn,1);	//set lower priority for adc
	am_hal_adc_sample_t Sample;
	conversionFinished = false;
	while(!conversionFinished){
		am_util_delay_us(5000);
		am_hal_adc_sw_trigger(g_ADCHandle);
	}
	//ADC->INTSET = 0x01;		//trigger the interrupt for test purpose
	//wait until interrupt is executed
	//while(!conversionFinished);

	uint32_t    ui32NumSamples = 1;
//	//while ( AM_HAL_ADC_FIFO_COUNT(ADC->FIFO) ){
		if (AM_HAL_STATUS_SUCCESS != am_hal_adc_samples_read(g_ADCHandle, false, NULL, &ui32NumSamples, &Sample)){
			am_util_stdio_printf("Error reading ADC value");
		}		
//	//}
	
	adc_deconfig();
	turnOffVDiv();
	return Sample.ui32Sample;
}

#ifdef TIMER3CONTROLLED
//*****************************************************************************
//
// Enable the ADC INIT TIMER 3A function and set for 0.5 second period.
//
//*****************************************************************************
static void
timer3a_init(void)
{
//
// Only CTIMER 3 supports the ADC.
//
#define TIMERNUM    3
	#ifdef PAM_DMA
	uint32_t ui32Period = 2500; //Set for 125 µseconds (8000 Hz) period
	#else
    uint32_t ui32Period = 2500; // Set for 50 µsecond (20000 Hz) period
	#endif

    //
    // Using the HFRC, no other clock source needs to be enabled
    //
    //am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_LFRC_START, 0);

    //
    // Set up timer 3A so start by clearing it.
    //
    am_hal_ctimer_clear(TIMERNUM, AM_HAL_CTIMER_TIMERA);

    //
    // Configure the timer to count 3MHz HFRC clocks but don't start it yet.
    //
    am_hal_ctimer_config(TIMERNUM, &g_sTimer3);
    //am_hal_ctimer_config_single(TIMERNUM,AM_HAL_CTIMER_TIMERA,(AM_HAL_CTIMER_HFRC_3MHZ | AM_HAL_CTIMER_FN_PWM_ONCE));
    //
    // Compute CMPR value needed for desired period based on a 3MHz clock.
    //
    ui32Period = ui32Period * 3 - 2;	//basically, first set period based on 1 MHz and then multiply by 3 to get corresponding value for 3 MHz, semi-calibrated for 20 kHz
    am_hal_ctimer_period_set(TIMERNUM, AM_HAL_CTIMER_TIMERA,
                             ui32Period, (ui32Period >> 1));

#if 0
    //
//  // Enable the timer output "pin". This refers to the pin as seen from
//  // inside the timer. The actual GPIO pin is neither enabled nor driven.
//	am_hal_ctimer_pin_enable(TIMERNUM, AM_HAL_CTIMER_TIMERA);
#endif
#if 1
    //
    // CTimer A3 is available on the following pads: 5, 22, 31, 43, 48, 37.
    // On the apollo3_evb, only pin 31 is unused, so we'll pick it.
    //
	am_hal_gpio_pinconfig(48,pad2FNCsel);	//set pin to ct28
    am_hal_ctimer_output_config(TIMERNUM, AM_HAL_CTIMER_TIMERA, 48,
                                AM_HAL_CTIMER_OUTPUT_NORMAL,
                                AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA);
#endif

    //
    // Set up timer 3A as the trigger source for the ADC.
    //
    am_hal_ctimer_adc_trigger_enable();

#if 0
    //
    // Clear the timer Interrupt
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0 << (TIMERNUM * 2));
#endif

    //
    // Start timer 3A.
    //
    am_hal_ctimer_start(TIMERNUM, AM_HAL_CTIMER_TIMERA);
} // timer_init()
#endif

void
sleep(void)
{

    //
    // Go to Deep Sleep.
    //
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);

}

void takePAMsamples(uint32_t sampleBuffer[LEN]){
	adc_init_PAM((void * )sampleBuffer);
	NVIC_EnableIRQ(ADC_IRQn);
adcValue = 0;
	am_hal_interrupt_master_enable();
	
	uint16_t sampleCounter = 0;
	uint32_t ui32NumSamples = 1;
	am_hal_adc_sample_t Sample;
#ifdef PAM_DMA
	while(!g_bADCDMAComplete){
		adcTriggerInterrupt();
		//
		// Reset the DMA completion and error flags.
		//
        if (!g_bADCDMAComplete)
        {
            sleep();
//			__NOP();
        }
		
		
	}
#else
	adcTriggerInterrupt();
	while(sampleCounter < LEN){
		while ( AM_HAL_ADC_FIFO_COUNT(ADC->FIFO) ){
			am_hal_adc_samples_read(g_ADCHandle, true, NULL, &ui32NumSamples, &Sample);
			if(Sample.ui32Sample > 4096){
				sampleBuffer[sampleCounter] = Sample.ui32Sample >> 6;
				//turnOff33();
			}
			else{
				sampleBuffer[sampleCounter] = Sample.ui32Sample;
				//turnOff33();
			}
			sampleCounter++;
			if(sampleCounter == LEN){
				break;
			}
		}
	}
#endif
	adc_deconfig();	//turn off ADC
	am_hal_ctimer_stop(3,AM_HAL_CTIMER_TIMERA);	//stop Timer for sampling
}