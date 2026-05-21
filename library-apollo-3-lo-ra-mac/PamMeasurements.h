

#ifndef PAM_MEASUREMENTS_H
#define PAM_MEASUREMENTS_H

#include <stdint.h>
#include <stdbool.h>


// ADC Buffer size
//#define ADC_SAMPLE_BUF_SIZE 482

// Function declare
void initPamMeasurement(void);
void DoPamMeasurement(float *avg_ml, float *avg_sp, float *yield);


#endif // PAM_MEASUREMENTS_H