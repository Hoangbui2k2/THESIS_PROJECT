#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Định nghĩa các thông số ADC
#define LIGHT_SENSOR_ADC_CHANNEL ADC1_CHANNEL_6 // Ví dụ: GPIO34
#define LIGHT_SENSOR_ADC_WIDTH ADC_WIDTH_BIT_12
#define LIGHT_SENSOR_ADC_ATTEN ADC_ATTEN_DB_0

// Định nghĩa hằng số để chuyển đổi ADC thành Lux
#define ADC_MAX_VALUE 4095         // 12-bit ADC
#define LUX_CALIBRATION_FACTOR 0.5 // Thay đổi dựa trên cảm biến cụ thể

// Hàm khởi tạo cảm biến ánh sáng
void light_sensor_init(void);

// Hàm đọc giá trị ánh sáng trả về đơn vị Lux
int light_sensor_read_lux(void);

#endif // LIGHT_SENSOR_H