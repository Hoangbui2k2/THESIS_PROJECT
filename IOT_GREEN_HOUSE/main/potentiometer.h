#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include "driver/adc.h"

// ADC channel for potentiometer
#define ADC_POTENTIOMETER ADC1_CHANNEL_4 // GPIO32 (ADC1 channel 4)

// Global variable for phase delay (defined in dimmer.c)
extern volatile int phase_delay;

// Function declarations
void potentiometer_init(void);
void potentiometer_update(void);

#endif // POTENTIOMETER_H
