/****************************************************************************
 * PAM Measurements
 * --------------------------------------------------------------------------
 ****************************************************************************/

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "timing.h"

//Pin definitions for my GPIO write func

#define PIN11  (1 << 11)    


#define PIN8   (1 << 8)    

#define PIN40   (1 << (40 - 32))     //HLD 12

#define PIN39   (1 << (39 - 32))     //RST 13

#define PIN38   (1 << (38 - 32))     //ML 11



#define PIN42 (1 << (42 - 32))    //SP 9


#define PIN36 (1 << (36 - 32))    //SW1 

#define PIN37 (1 << (37 - 32))    //SW2 



//New GPIOS: ML=11, SP=8, SW2=7, SW1=6, HLD=12, Res=13
//A5 ADC

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

#pragma region ADC

#define ADC_EXAMPLE_DEBUG   1

// ADC Sample buffer. A circular buffer to hold the ADC samples
#define ADC_SAMPLE_BUF_SIZE 700

uint32_t index;
uint32_t g_ui32ADCSampleBuffer[ADC_SAMPLE_BUF_SIZE];
am_hal_adc_sample_t SampleBuffer[ADC_SAMPLE_BUF_SIZE * 5];

// ADC Device Handle.
static void* g_ADCHandle;

// ADC DMA complete flag.
volatile bool                   g_bADCDMAComplete;

// ADC DMA error flag.
volatile bool                   g_bADCDMAError;

// Define the ADC SE0 pin to be used.
const am_hal_gpio_pincfg_t g_AM_PIN_35_ADCSE7 =
{
    .uFuncSel = AM_HAL_PIN_35_ADCSE7,
};


// Interrupt handler for the ADC.
void am_adc_isr(void)
{
    uint32_t ui32IntMask;

    // Read the interrupt status.
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_status(g_ADCHandle, &ui32IntMask, false))
    {
        am_util_stdio_printf("Error reading ADC interrupt status\n");
    }

    // Clear the ADC interrupt.
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_clear(g_ADCHandle, ui32IntMask))
    {
        am_util_stdio_printf("Error clearing ADC interrupt status\n");
    }

    am_util_stdio_printf("Entered\n");
    // If we got a DMA complete, set the flag.
    if (ui32IntMask & AM_HAL_ADC_INT_DCMP)
    {
        g_bADCDMAComplete = true;
    }

    // If we got a DMA error, set the flag.
    if (ui32IntMask & AM_HAL_ADC_INT_DERR)
    {
        g_bADCDMAError = true;
    }
}

// Configure the ADC.
void adc_config_dma(void)
{
    am_hal_adc_dma_config_t       ADCDMAConfig;

    // Configure the ADC to use DMA for the sample transfer.
    ADCDMAConfig.bDynamicPriority = true;
    ADCDMAConfig.ePriority = AM_HAL_ADC_PRIOR_SERVICE_IMMED;
    ADCDMAConfig.bDMAEnable = true;
    ADCDMAConfig.ui32SampleCount = ADC_SAMPLE_BUF_SIZE;
    ADCDMAConfig.ui32TargetAddress = (uint32_t)g_ui32ADCSampleBuffer;

    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_dma(g_ADCHandle, &ADCDMAConfig))
    {
        am_util_stdio_printf("Error - configuring ADC DMA failed.\n");
    }

    // Reset the ADC DMA flags.
    g_bADCDMAComplete = false;
    g_bADCDMAError = false;
}

// Configure the ADC.
void adc_config(void)
{
    am_hal_adc_config_t           ADCConfig;
    am_hal_adc_slot_config_t      ADCSlotConfig;

    // Initialize the ADC and get the handle.
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_initialize(0, &g_ADCHandle))
    {
        am_util_stdio_printf("Error - reservation of the ADC instance failed.\n");
    }

    // Power on the ADC.
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
        AM_HAL_SYSCTRL_WAKE,
        false))
    {
        am_util_stdio_printf("Error - ADC power on failed.\n");
    }

    // Set up the ADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    ADCConfig.eClock = AM_HAL_ADC_CLKSEL_HFRC;     //Why is divided?
    ADCConfig.ePolarity = AM_HAL_ADC_TRIGPOL_RISING;
    ADCConfig.eTrigger = AM_HAL_ADC_TRIGSEL_SOFTWARE;
    ADCConfig.eReference = AM_HAL_ADC_REFSEL_INT_2P0;
    ADCConfig.eClockMode = AM_HAL_ADC_CLKMODE_LOW_LATENCY;
    ADCConfig.ePowerMode = AM_HAL_ADC_LPMODE0;      // CHECK HERE
    ADCConfig.eRepeat = AM_HAL_ADC_REPEATING_SCAN;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure(g_ADCHandle, &ADCConfig))
    {
        am_util_stdio_printf("Error - configuring ADC failed.\n");
    }

    // Set up an ADC slot
    ADCSlotConfig.eMeasToAvg = AM_HAL_ADC_SLOT_AVG_1;      //What is this?
    ADCSlotConfig.ePrecisionMode = AM_HAL_ADC_SLOT_10BIT;
    ADCSlotConfig.eChannel = AM_HAL_ADC_SLOT_CHSEL_SE7;   //A4 for redboard artemis nano
    ADCSlotConfig.bWindowCompare = false;
    ADCSlotConfig.bEnabled = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 0, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 0 failed.\n");
    }

}

// Initialize the ADC repetitive sample timer A3.
void init_timerA3_for_ADC(void)
{

    // Start a timer to trigger the ADC periodically (1 second).
    am_hal_ctimer_config_single(3, AM_HAL_CTIMER_TIMERA,
        AM_HAL_CTIMER_HFRC_12MHZ |
        AM_HAL_CTIMER_FN_REPEAT |
        AM_HAL_CTIMER_INT_ENABLE);

    //am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA3);

    am_hal_ctimer_period_set(3, AM_HAL_CTIMER_TIMERA, 10, 5);

    // Enable the timer A3 to trigger the ADC directly
    am_hal_ctimer_adc_trigger_enable();

    // Start the timer.
    //am_hal_ctimer_start(3, AM_HAL_CTIMER_TIMERA);
}

void adc_start(void)
{
    // Configure the ADC to use DMA for the sample transfer.
    adc_config_dma();

    // Clear the DMA done flag if you use one 
    g_bADCDMAComplete = false;
    
    // 4) Trigger the ADC if single-scan or software-trigger
    am_hal_adc_sw_trigger(g_ADCHandle);

     //
    // For this example, the samples will be coming in slowly. This means we
    // can afford to wake up for every conversion.
    //
    am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_DERR | AM_HAL_ADC_INT_DCMP);

    
    // Enable the ADC.
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_enable(g_ADCHandle))
    {
        am_util_stdio_printf("Error - enabling ADC failed.\n");
    }

}

void adc_deconfig(void)
{
    // Disable ADC interrupts if you want
    //am_hal_adc_interrupt_disable(g_ADCHandle, AM_HAL_ADC_INT_DMACPL);

    // Disable the ADC
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_disable(g_ADCHandle))
	{
        am_util_stdio_printf("Error -disable");

    }
	//
	// Initialize the ADC and get the handle.
	//
	if (AM_HAL_STATUS_SUCCESS != am_hal_adc_deinitialize(g_ADCHandle))
	{
	 am_util_stdio_printf("Error - deinitialize.\n");
	}

}



#pragma endregion

/**
 *  Set and clear multiple GPIO pins in a single register write.
 *
 * setMask   Bitmask of pins to set HIGH.
 * clearMask Bitmask of pins to set LOW.
 *
 * Both setMask and clearMask can include multiple pins. For example,
 *   setMask   = (1 << 5) | (1 << 7);  // pins 5, 7 go HIGH
 *   clearMask = (1 << 11);           // pin 11 goes LOW
 *
 * Any pins not in (setMask | clearMask) remain unchanged.
 * 
 * Note: If a pin is in both setMask and clearMask, it ends up HIGH
 *       because we clear first, then set. Avoid overlapping bits
 *       unless you specifically want that final result.
 */
void gpio_A_write_multiple(uint32_t setMask, uint32_t clearMask)
{
    // 1. Read the current state of the GPIO outputs
    uint32_t currentVal = GPIO->WTA;

    // 2. Clear the bits we want to force LOW
    currentVal &= ~clearMask;

    // 3. Set the bits we want HIGH
    currentVal |= setMask;

    // 4. Write back in a single store. All changed bits update simultaneously
    GPIO->WTA = currentVal;
}


//GPIO 32-49
void gpio_B_write_multiple(uint32_t setMask, uint32_t clearMask)
{
    // 1. Read the current state of the GPIO outputs
    uint32_t currentVal = GPIO->WTB;

    // 2. Clear the bits we want to force LOW
    currentVal &= ~clearMask;

    // 3. Set the bits we want HIGH
    currentVal |= setMask;

    // 4. Write back in a single store. All changed bits update simultaneously
    GPIO->WTB = currentVal;
}

//ML PIN38, SP38, RST PIN39, HLD PIN40, SW1 PIN36, SW2 PIN37
void MeasuringLight(void){
		//Open both tmux before integration so the capacitors don`t effect the integration
		gpio_B_write_multiple(0, PIN37| PIN36); 

		gpio_B_write_multiple(PIN39, PIN40);     //Reset switch open(1), Hold close(0) 
		am_util_delay_us(7);	  //10 us integrate

		gpio_B_write_multiple(PIN39|PIN40, 0); // Both Open, integration stops

		//Tmux switch control 1
				//Close switch so it flows to opamp
				//Open switch so break the op amp connection from integrator
		gpio_B_write_multiple(PIN36, PIN37);
		am_util_delay_us(9);
		gpio_B_write_multiple(0, PIN36| PIN37);     //Open both switches before next integ.
		 //Here Hold is open, so the capacitor in pd starts to charge and when hold closed, it releases.
		 //Reset the current building in photodiode before the Integration2 starts.

		gpio_B_write_multiple(0, PIN39|PIN40); //Both closed, resets

		am_util_delay_us(7);	  //10 us enough for reset

		//Integrate2 (Integration during the ML with LED ON)
		gpio_B_write_multiple(PIN39 | PIN38, PIN40);   //Hold closed(0), Reset open(1), LED on(1)

		am_util_delay_us(7);	  //10 us integrate

		//Hold2
		gpio_B_write_multiple( PIN40 | PIN39, PIN38);   //Hold open(1), Reset open(1), LED off(0)

		//Tmux switch control 2
		gpio_B_write_multiple(PIN37, PIN36);
		am_util_delay_us(9);
		gpio_B_write_multiple(0, PIN36| PIN37);     //Open both switches.


		am_util_delay_us(56);  //After LED turns off, Hold both values for a while to collect the data.


		am_hal_ctimer_start(3, AM_HAL_CTIMER_TIMERA);

		am_util_delay_us(7);  //After LED turns off, Hold both values for a while to collect the data.
		am_hal_ctimer_stop(3, AM_HAL_CTIMER_TIMERA);
		am_hal_ctimer_clear(3, AM_HAL_CTIMER_TIMERA);

		am_util_delay_us(2);  //
		//
		// 30 us wait after integrations, and 20 samples for each

		gpio_B_write_multiple(0, PIN40| PIN39); //Reset the current building in photodiode
		gpio_B_write_multiple(PIN36| PIN37, 0); //Also reset tmux
}

void SaturationPulse(void){

    gpio_B_write_multiple(PIN42, 0); // Start LED off
    my_delay_us_xtal(9460);
		am_util_delay_us(13);
    gpio_B_write_multiple(0, PIN42); // Start LED off
    
}


//---------------------------------------------------------------------------
// MAIN
//---------------------------------------------------------------------------
void initPamMeasurement(void){
    delay_init_xtal_one_shot();


    //
    // 5. Enable the 32.768 kHz crystal.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_START, 0);

    // Start the CTIMER A3 for timer-based ADC measurements.
    init_timerA3_for_ADC();
    NVIC_EnableIRQ(ADC_IRQn);
    NVIC_EnableIRQ(CTIMER_IRQn);

    am_hal_interrupt_master_enable();

    // Set a pin to act as our ADC input (Redboard Artemis - Pin A4)
    am_hal_gpio_pinconfig(35, g_AM_PIN_35_ADCSE7);

    // Configure the ADC
    adc_config();

    adc_start();
    // Trigger the ADC sampling for the first time manually.
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_sw_trigger(g_ADCHandle))
    {
        am_util_stdio_printf("Error - triggering the ADC failed.\n");
    }

    //
    // 6. Enable CTIMER interrupts at the NVIC level.
    //


    //
    // 7. Configure the GPIO pins as output
    //
    am_hal_gpio_pinconfig(40, g_AM_HAL_GPIO_OUTPUT); // LED pin -- REDBOARD 12
    am_hal_gpio_state_write(40, AM_HAL_GPIO_OUTPUT_CLEAR); // Start LED off

    am_hal_gpio_pinconfig(39, g_AM_HAL_GPIO_OUTPUT); // LED pin -- REDBOARD 11
    am_hal_gpio_state_write(39, AM_HAL_GPIO_OUTPUT_CLEAR); // Start LED off

    am_hal_gpio_pinconfig(38, g_AM_HAL_GPIO_OUTPUT); // LED pin -- REDBOARD 11
    am_hal_gpio_state_write(38, AM_HAL_GPIO_OUTPUT_CLEAR); // Start LED off

    am_hal_gpio_pinconfig(42, g_AM_HAL_GPIO_OUTPUT); //8--REDBOARD
    am_hal_gpio_state_write(42, AM_HAL_GPIO_OUTPUT_CLEAR); // Start LED off

    am_hal_gpio_pinconfig(39, g_AM_HAL_GPIO_OUTPUT); //8--REDBOARD
    am_hal_gpio_state_write(39, AM_HAL_GPIO_OUTPUT_CLEAR); // Start LED off

    am_hal_gpio_pinconfig(37, g_AM_HAL_GPIO_OUTPUT); //8--REDBOARD
    am_hal_gpio_state_write(37, AM_HAL_GPIO_OUTPUT_CLEAR); // Start LED off

    am_hal_gpio_pinconfig(36, g_AM_HAL_GPIO_OUTPUT); //8--REDBOARD
    am_hal_gpio_state_write(36, AM_HAL_GPIO_OUTPUT_CLEAR); // Start LED off


}




void DoPamMeasurement(float *avg_ml, float *avg_sp, float *yield)
{
		float total_ml, total_sp;

    #pragma region while
    int i, j;
    //ML Cycle
    for(i = 0; i<10; i++){

        //Each pulse has 12 samples.
        MeasuringLight();
        my_delay_us_xtal(99880);
        //---debug adc---
        /*
        my_delay_us_xtal(140);  //Complete 500 us before next SP
        am_hal_gpio_state_write(6, AM_HAL_GPIO_OUTPUT_SET); // Start LED on
        am_hal_ctimer_start(3, AM_HAL_CTIMER_TIMERA);
        am_util_delay_us(7);  //After LED turns off, Hold both values for a while to collect the data.
        am_hal_ctimer_stop(3, AM_HAL_CTIMER_TIMERA);
        am_hal_ctimer_clear(3, AM_HAL_CTIMER_TIMERA);
        am_hal_gpio_state_write(6, AM_HAL_GPIO_OUTPUT_CLEAR); // Start LED off
        */
    }

    //SP Cycle
    for(j = 0; j<50; j++){
        SaturationPulse();
        my_delay_us_xtal(100);
        MeasuringLight();  //Takes around 137 us
        my_delay_us_xtal(120);  //Complete 500 us before next SP
        //---debug adc---
        /*
        am_hal_gpio_state_write(5, AM_HAL_GPIO_OUTPUT_SET); // Start LED on
        my_delay_us_xtal(140);  //Complete 500 us before next SP
        am_hal_ctimer_start(3, AM_HAL_CTIMER_TIMERA);
        am_util_delay_us(7);  //After LED turns off, Hold both values for a while to collect the data.
        am_hal_ctimer_stop(3, AM_HAL_CTIMER_TIMERA);
        am_hal_ctimer_clear(3, AM_HAL_CTIMER_TIMERA);
        am_hal_gpio_state_write(5, AM_HAL_GPIO_OUTPUT_CLEAR); // Start LED on
*/
    }

    while (1)
    {
        //SaturationPulse();
        //gpio_A_write_multiple(PIN38, PIN40);
        //my_delay_ms_xtal(100);

        //gpio_B_write_multiple(PIN40, PIN38);
        my_delay_ms_xtal(100);

    if (g_bADCDMAError)
				{
						am_util_stdio_printf("DMA Error occured\n");
						*avg_ml = 5;
						*avg_sp = 5;
						*yield = 1;
						break;
				}

        // Check if the ADC DMA completion interrupt occurred.
    if (g_bADCDMAComplete)
        {
            uint32_t        ui32SampleCount, SampleBufferCount;
            am_util_stdio_printf("DMA Complete\n");
            uint32_t output_offset = 0;     //Shift the new samples in big Sample Buffer
            ui32SampleCount = ADC_SAMPLE_BUF_SIZE;
            SampleBufferCount = ADC_SAMPLE_BUF_SIZE * 10;
            if (AM_HAL_STATUS_SUCCESS != am_hal_adc_samples_read(g_ADCHandle, false,
                            g_ui32ADCSampleBuffer,
                            &ui32SampleCount,
                            &SampleBuffer[output_offset]))
                        {
                            am_util_stdio_printf("Error - failed to process samples.\n");
                        }
						else{
							
							total_ml = 0;
							total_sp = 0;
							// Print the buffer contents
							for (uint32_t n = 13; n < ui32SampleCount; n++)
							{
									//am_util_stdio_printf("S[%d] %d\r\n", n, SampleBuffer[n]);
									//am_util_delay_ms(6);
									if ( n < 109)
											total_ml = total_ml + SampleBuffer[n].ui32Sample;
									else if (n >= 241)
											total_sp = total_sp + SampleBuffer[n].ui32Sample;
							}
							*avg_ml = total_ml / (109 - 13);
							*avg_sp = total_sp / (ui32SampleCount - 241);

							*avg_ml = (*avg_ml * 2) / 1024;
							*avg_sp = (*avg_sp * 2) / 1024;
							
							
							*avg_ml = *avg_ml - 0.035; 
							*avg_sp = *avg_sp - 0.035; 

							am_util_delay_ms(5);
							am_util_stdio_printf("Average ML %f\n", *avg_ml);
							am_util_delay_ms(5);
							am_util_stdio_printf("Average SP: %f\n", *avg_sp);
							am_util_delay_ms(5);
							*yield = (*avg_sp - *avg_ml) / *avg_sp;
							am_util_stdio_printf("Yield: %f\n", *yield);


							 // Reset the DMA completion and error flags.
							g_bADCDMAComplete = false;

							// Re-configure the ADC DMA.
							adc_config_dma();

							// Clear the ADC interrupts.
							if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_clear(g_ADCHandle, 0xFFFFFFFF))
							{
									am_util_stdio_printf("Error - clearing the ADC interrupts failed.\n");
							}

							adc_deconfig();
					}
            break;


            // Trigger the ADC sampling for the first time manually.
            if (AM_HAL_STATUS_SUCCESS != am_hal_adc_sw_trigger(g_ADCHandle))
            {
                am_util_stdio_printf("Error - triggering the ADC failed.\n");
            }

        } // if ()
    
    }
#pragma endregion
}

