#include "driver/i2c.h"
#include "display_oled.h"
#include "ssd1306.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include <string.h>

static SSD1306_t dev;

void init_oled(SSD1306_t *dev)
{
    i2c_master_init(dev, 21, 22, -1); // Set I2C pins
    ssd1306_init(dev, 128, 64);       // Initialize display with width 128 and height 64
    ssd1306_clear_screen(dev, false);
    ssd1306_contrast(dev, 0xFF);      // Set maximum contrast
}

void update_oled_display(int light_level, float soil_moisture, int temperature, int humidity) {
    char buffer[20];

    // Display temperature
    snprintf(buffer, sizeof(buffer), "Temp: %d C", temperature);
    ssd1306_display_text(&dev, 0, buffer, strlen(buffer), false);

    // Display humidity
    snprintf(buffer, sizeof(buffer), "Humid: %d %%", humidity);
    ssd1306_display_text(&dev, 1, buffer, strlen(buffer), false);

    // Display soil moisture
    snprintf(buffer, sizeof(buffer), "Soil: %.2f %%", soil_moisture);
    ssd1306_display_text(&dev, 2, buffer, strlen(buffer), false);

    // Display light level
    snprintf(buffer, sizeof(buffer), "Light: %d lux", light_level);
    ssd1306_display_text(&dev, 3, buffer, strlen(buffer), false);

    // Update OLED display
    ssd1306_show_buffer(&dev);
}
