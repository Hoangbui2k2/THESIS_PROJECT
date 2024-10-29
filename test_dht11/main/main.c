#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht11.h"
#include "esp_log.h"

static const char *TAG = "DHT11_APP";

void dht11_task(void *pvParameter)
{
    dht11_data_t dht11_data;
    while (1)
    {
        if (DHT11_read_data(&dht11_data) == ESP_OK)
        {
            ESP_LOGI(TAG, "Temperature: %d°C, Humidity: %d%%", dht11_data.temperature, dht11_data.humidity);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read data from DHT11 sensor");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));  // Wait for 2 seconds before next reading
    }
}

void app_main(void)
{
    xTaskCreate(&dht11_task, "dht11_task", 4096, NULL, 5, NULL);
    
}
