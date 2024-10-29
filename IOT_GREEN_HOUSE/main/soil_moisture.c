#include "soil_moisture.h"
#include "driver/adc.h"
#include "esp_log.h"

// Logger tag
static const char *TAG = "SOIL_MOISTURE";

// Hàm khởi tạo cấu hình ADC
void soil_moisture_init(void)
{
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
}

// Hàm đọc giá trị độ ẩm đất và trả về theo phần trăm
float read_soil_moisture(void)
{
    // Đọc giá trị ADC
    int adc_value = adc1_get_raw(ADC_CHANNEL);

    // Chuyển đổi giá trị ADC thành điện áp
    float voltage = adc_value * (3.3 / 4095.0); // Giả định điện áp tham chiếu là 3.3V

    // Chuyển đổi điện áp thành phần trăm độ ẩm đất
    float moisture_percentage = ((3.3-voltage) / 3.3) * 100; // Tỉ lệ độ ẩm dựa trên 3.3V

    // Log giá trị độ ẩm
    // ESP_LOGI(TAG, "ADC Value: %d", adc_value);
    // ESP_LOGI(TAG, "Voltage: %.2f V", voltage);
    // ESP_LOGI(TAG, "Soil Moisture: %.2f%%", moisture_percentage);

    return moisture_percentage;
}
