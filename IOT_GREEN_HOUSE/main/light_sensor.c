#include "light_sensor.h"
#include "driver/adc.h"
#include "esp_log.h"

// Logger tag
static const char *TAG = "light_sensor";

// Hàm khởi tạo cấu hình ADC
void light_sensor_init(void)
{
    adc1_config_width(LIGHT_SENSOR_ADC_WIDTH);
    adc1_config_channel_atten(LIGHT_SENSOR_ADC_CHANNEL, LIGHT_SENSOR_ADC_ATTEN);
}

// Hàm đọc giá trị ánh sáng trả về đơn vị Lux
int light_sensor_read_lux(void)
{
    // Đọc giá trị ADC
    int adc_reading = adc1_get_raw(LIGHT_SENSOR_ADC_CHANNEL);

    // Chuyển đổi giá trị ADC thành Lux
    int lux_value = adc_reading;

    // Log giá trị Lux
    // ESP_LOGI(TAG, "Lux Reading: %d", lux_value);

    return lux_value;
}