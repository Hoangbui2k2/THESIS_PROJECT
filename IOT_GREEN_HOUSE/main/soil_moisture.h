#ifndef SOIL_MOISTURE_H
#define SOIL_MOISTURE_H

// Định nghĩa các thông số ADC
#define ADC_CHANNEL ADC1_CHANNEL_7 // Ví dụ: GPIO35
#define ADC_WIDTH ADC_WIDTH_BIT_12
#define ADC_ATTEN ADC_ATTEN_DB_11

// Hàm khởi tạo cảm biến độ ẩm đất
void soil_moisture_init(void);

// Hàm đọc và trả về giá trị độ ẩm đất (%)
float read_soil_moisture(void);

#endif // SOIL_MOISTURE_H