#include "control_devices.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "CONTROL_DEVICES";

// Initialize GPIOs for controlling devices
void control_devices_init(void)
{
    esp_rom_gpio_pad_select_gpio(WATER_PUMP_GPIO_PIN);
    gpio_set_direction(WATER_PUMP_GPIO_PIN, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(MIST_PUMP_GPIO_PIN);
    gpio_set_direction(MIST_PUMP_GPIO_PIN, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(EXHAUST_FAN_GPIO_PIN);
    gpio_set_direction(EXHAUST_FAN_GPIO_PIN, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(BLOWER_FAN_GPIO_PIN);
    gpio_set_direction(BLOWER_FAN_GPIO_PIN, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(LIGHT_GPIO_PIN);
    gpio_set_direction(LIGHT_GPIO_PIN, GPIO_MODE_OUTPUT);
}

// Update device states based on sensor readings
void control_devices_update(float light_level, float soil_moisture, uint8_t temperature, uint8_t humidity)
{
    // Control light based on light sensor
    if (light_level < LIGHT_THRESHOLD_LUX)
    {
        gpio_set_level(LIGHT_GPIO_PIN, 0); // Turn on light
        ESP_LOGI(TAG, "Light ON: Light level is below threshold.");
    }
    else
    {
        gpio_set_level(LIGHT_GPIO_PIN, 1); // Turn off light
        ESP_LOGI(TAG, "Light OFF: Light level is above threshold.");
    }

    // Control water pump based on soil moisture sensor
    if (soil_moisture < SOIL_MOISTURE_THRESHOLD)
    {
        gpio_set_level(WATER_PUMP_GPIO_PIN, 0); // Turn on water pump
        ESP_LOGI(TAG, "Water Pump ON: Soil moisture is below threshold.");
    }
    else
    {
        gpio_set_level(WATER_PUMP_GPIO_PIN, 1); // Turn off water pump
        ESP_LOGI(TAG, "Water Pump OFF: Soil moisture is above threshold.");
    }

    // Control mist pump based on humidity
    if (humidity < HUMIDITY_THRESHOLD)
    {
        gpio_set_level(MIST_PUMP_GPIO_PIN, 0); // Turn on mist pump
        ESP_LOGI(TAG, "Mist Pump ON: Humidity is below threshold.");
    }
    else
    {
        gpio_set_level(MIST_PUMP_GPIO_PIN, 1); // Turn off mist pump
        ESP_LOGI(TAG, "Mist Pump OFF: Humidity is above threshold.");
    }

    // Control exhaust fan based on temperature
    if (temperature > TEMPERATURE_THRESHOLD)
    {
        gpio_set_level(EXHAUST_FAN_GPIO_PIN, 0); // Turn on exhaust fan
        ESP_LOGI(TAG, "Exhaust Fan ON: Temperature is above threshold.");
    }
    else
    {
        gpio_set_level(EXHAUST_FAN_GPIO_PIN, 1); // Turn off exhaust fan
        ESP_LOGI(TAG, "Exhaust Fan OFF: Temperature is below threshold.");
    }

    // Control blower fan based on temperature and humidity
    if (temperature > TEMPERATURE_THRESHOLD || humidity > HUMIDITY_THRESHOLD)
    {
        gpio_set_level(BLOWER_FAN_GPIO_PIN, 0); // Turn on blower fan
        ESP_LOGI(TAG, "Blower Fan ON: Temperature or humidity is above threshold.");
    }
    else
    {
        gpio_set_level(BLOWER_FAN_GPIO_PIN, 1); // Turn off blower fan
        ESP_LOGI(TAG, "Blower Fan OFF: Temperature and humidity are below threshold.");
    }
}
