#ifndef DHT11_H
#define DHT11_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"

typedef struct
{
    uint8_t temperature;
    uint8_t humidity;
} dht11_data_t;

// Hàm khởi tạo cảm biến DHT11
void DHT11_init(void);

// Hàm đọc dữ liệu từ DHT11 và trả về nhiệt độ và độ ẩm
esp_err_t DHT11_read(dht11_data_t *data);

#endif // DHT11_H