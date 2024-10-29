#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

#define ADC_CHANNEL ADC1_CHANNEL_0 // GPIO36 (VP) if you are using ADC1 channel 0
#define ADC_WIDTH   ADC_WIDTH_BIT_12
#define ADC_ATTEN   ADC_ATTEN_DB_11

#define PUMP_GPIO_PIN GPIO_NUM_18 // Define the GPIO pin for the water pump
#define MOISTURE_THRESHOLD 40.0   // Define the threshold for soil moisture (in percentage)

static const char *TAG = "ADC_SOIL_MOISTURE";

// Function to read ADC value and convert to voltage
float read_adc_voltage(void) {
    int adc_value = adc1_get_raw(ADC_CHANNEL);
    float voltage = adc_value * (3.3 / 4095.0); // Assuming a 3.3V reference voltage
    ESP_LOGI(TAG, "ADC Value: %d", adc_value);
    ESP_LOGI(TAG, "Voltage: %.2f V", voltage);
    return voltage;
}

// Task to read soil moisture and control the water pump
void soil_moisture_task(void *pvParameter) {
    esp_rom_gpio_pad_select_gpio(PUMP_GPIO_PIN);
    gpio_set_direction(PUMP_GPIO_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        float voltage = read_adc_voltage();
        float moisture_percentage = (voltage / 3.3) * 100; // Convert voltage to percentage
        ESP_LOGI(TAG, "Soil Moisture: %.2f%%", moisture_percentage);

        if (moisture_percentage < MOISTURE_THRESHOLD) {
            gpio_set_level(PUMP_GPIO_PIN, 1); // Turn on the pump
            ESP_LOGI(TAG, "Pump ON");
        } else {
            gpio_set_level(PUMP_GPIO_PIN, 0); // Turn off the pump
            ESP_LOGI(TAG, "Pump OFF");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

void app_main(void) {
    // Configure ADC
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    // Create the soil moisture task
    xTaskCreate(&soil_moisture_task, "soil_moisture_task", 2048, NULL, 5, NULL);
}
