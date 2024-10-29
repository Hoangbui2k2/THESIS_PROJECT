#ifndef DIMMER_H
#define DIMMER_H

#include "driver/gpio.h"

// GPIO definitions
#define GPIO_ZERO_CROSS 14               // GPIO for zero-cross detection
#define GPIO_TRIAC_CONTROL 27            // GPIO for TRIAC control

// Global variable for phase delay (microseconds)
extern volatile int phase_delay;

// Function declarations
void dimmer_init(void);
void dimmer_set_phase_delay(int delay);
void IRAM_ATTR zero_cross_isr_handler(void *arg);

#endif // DIMMER_H
