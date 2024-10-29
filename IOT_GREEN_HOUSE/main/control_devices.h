#ifndef CONTROL_DEVICES_H
#define CONTROL_DEVICES_H

#include "esp_err.h"

// GPIO Pins for controlling devices
#define WATER_PUMP_GPIO_PIN GPIO_NUM_19
#define MIST_PUMP_GPIO_PIN GPIO_NUM_17
#define EXHAUST_FAN_GPIO_PIN GPIO_NUM_18
#define BLOWER_FAN_GPIO_PIN GPIO_NUM_5
#define LIGHT_GPIO_PIN GPIO_NUM_23

// Threshold values
#define LIGHT_THRESHOLD_LUX 100      // Threshold for light sensor
#define SOIL_MOISTURE_THRESHOLD 40.0 // Threshold for soil moisture percentage
#define TEMPERATURE_THRESHOLD 30     // Threshold for temperature (in Celsius)
#define HUMIDITY_THRESHOLD 50        // Threshold for humidity (in percentage)

// Function prototypes
void control_devices_init(void);
void control_devices_update(float light_level, float soil_moisture, uint8_t temperature, uint8_t humidity);

#endif // CONTROL_DEVICES_H