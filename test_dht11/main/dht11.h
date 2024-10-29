#ifndef DHT11_H_
#define DHT11_H_

#include "driver/gpio.h"
#include "esp_err.h"

typedef struct {
    uint8_t temperature;
    uint8_t humidity;
} dht11_data_t;

esp_err_t DHT11_read_data(dht11_data_t *data);

#endif // DHT11_H_
