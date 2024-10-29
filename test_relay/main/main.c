#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define PIN 27

void app_main(void)
{
    // Configure the GPIO pin as output
    esp_rom_gpio_pad_select_gpio(PIN);
    gpio_set_direction(PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN, 0); // Set pin to LOW

    while (1) {
        vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay for 3 seconds

        // Configure the GPIO pin as input
        gpio_set_direction(PIN, GPIO_MODE_INPUT);

        vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay for 3 seconds

        // Configure the GPIO pin as output again and set it to LOW
        gpio_set_direction(PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(PIN, 0);
    }
}
