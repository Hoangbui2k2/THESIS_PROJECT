#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "control_devices.h"
#include "light_sensor.h"  // Include file header cho cảm biến ánh sáng
#include "soil_moisture.h" // Include file header cho cảm biến độ ẩm đất
#include "dht11.h"         // Include file header cho cảm biến DHT11
#include "dimmer.h"
#include "ssd1306.h"
#include <string.h>
#include "driver/i2c.h"
#include "mqtt_client.h"
#include "potentiometer.h"
static const char *TAG = "MAIN";
void init_oled(SSD1306_t *dev)
{
    i2c_master_init(dev, 21, 22, -1); // Adjust SDA_PIN, SCL_PIN, RESET_PIN as per your setup
    i2c_init(dev, 128, 64);           // Initialize display with width 128 and height 64
    ssd1306_clear_screen(dev, false);
    ssd1306_contrast(dev, 0xFF); // Set maximum contrast
}

void display_sensor_data(SSD1306_t *dev, float temperature, float humidity, int soil_moisture, int light_level)
{
    char buffer[20];

    // Hiển thị nhiệt độ
    snprintf(buffer, sizeof(buffer), "Temp: %.1f C", temperature);
    ssd1306_display_text(dev, 0, buffer, strlen(buffer), false);

    // Hiển thị độ ẩm không khí
    snprintf(buffer, sizeof(buffer), "Humid: %.1f %%", humidity);
    ssd1306_display_text(dev, 1, buffer, strlen(buffer), false);

    // Hiển thị độ ẩm đất
    snprintf(buffer, sizeof(buffer), "Soil: %d %%", soil_moisture);
    ssd1306_display_text(dev, 2, buffer, strlen(buffer), false);

    // Hiển thị mức ánh sáng
    snprintf(buffer, sizeof(buffer), "Light: %d lux", light_level);
    ssd1306_display_text(dev, 3, buffer, strlen(buffer), false);

    // Cập nhật màn hình
    ssd1306_show_buffer(dev);
}

void app_main(void)
{
    // Initialize oled
    SSD1306_t dev;
    ssd1306_init(&dev, 128, 64);
    init_oled(&dev);

    // Initialize devices
    control_devices_init();
    // Initialize sensors (nếu cần thiết)
    light_sensor_init();  // Khởi tạo cảm biến ánh sáng
    soil_moisture_init(); // Khởi tạo cảm biến độ ẩm đất
    DHT11_init();         // Khởi tạo cảm biến DHT11
    dimmer_init();

    while (1)
    {
        // Đọc giá trị cảm biến ánh sáng
        float light_level = light_sensor_read_lux();
        ESP_LOGI(TAG, "Light level (lux): %.2f", light_level);

        // Đọc giá trị cảm biến độ ẩm đất
        float soil_moisture = read_soil_moisture();
        ESP_LOGI(TAG, "Soil moisture: %.2f%%", soil_moisture);

        // Đọc giá trị cảm biến DHT11 (nhiệt độ và độ ẩm)
        dht11_data_t dht11_data;
        esp_err_t dht_status = DHT11_read(&dht11_data); // nhiệt độ đang lệch 0.7 độ

        if (dht_status == ESP_OK)
        {
            ESP_LOGI(TAG, "Temperature: %d°C, Humidity: %d%%", dht11_data.temperature, dht11_data.humidity);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read data from DHT11 sensor");
            dht11_data.temperature = 0; // Gán giá trị mặc định khi đọc lỗi
            dht11_data.humidity = 0;    // Gán giá trị mặc định khi đọc lỗi
        }

        // Display  on oled
        ssd1306_clear_screen(&dev, false);
        display_sensor_data(&dev, dht11_data.temperature, dht11_data.humidity, soil_moisture, light_level);

        // update value of potiemtion to dimmer
        potentiometer_update();

        // Update device states based on sensor readings
        control_devices_update(light_level, soil_moisture, dht11_data.temperature, dht11_data.humidity);
        // xTaskCreate(dimmer_task, "dimmer_task", 2048, NULL, 5, NULL);
        // Delay for a period (e.g., 5 seconds)

        vTaskDelay(pdMS_TO_TICKS(1000)); // 5 seconds delay
    }
}
