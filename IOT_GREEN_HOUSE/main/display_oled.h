#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "ssd1306.h"
// // Forward declaration of the sensor_data_t struct
// typedef struct {
//     int light_level;
//     float soil_moisture;
//     int temperature;
//     int humidity;
// } sensor_data_t;

// // External variables for FreeRTOS queue and semaphore
// extern QueueHandle_t sensor_data_queue;
// extern SemaphoreHandle_t sensor_data_mutex;

// Function prototypes
void init_oled(SSD1306_t *dev);
void update_oled_display(int light_level, float soil_moisture, int temperature, int humidity);
void display_oled_task(void *pvParameters);

#endif // DISPLAY_OLED_H
