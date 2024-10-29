#include "dht11.h"
#include "esp_log.h"

static const char *TAG = "DHT11";

#define DHT11_DATA_PIN GPIO_NUM_25 // Chân ngõ vào data DHT11

void DHT11_delay_us(uint32_t time)
{
    esp_rom_delay_us(time);
}

void DHT11_set_pin_output()
{
    gpio_set_direction(DHT11_DATA_PIN, GPIO_MODE_OUTPUT);
}

void DHT11_set_pin_input()
{
    gpio_set_direction(DHT11_DATA_PIN, GPIO_MODE_INPUT);
}

void DHT11_write_pin(uint8_t value)
{
    gpio_set_level(DHT11_DATA_PIN, value);
}

uint8_t DHT11_read_pin()
{
    return gpio_get_level(DHT11_DATA_PIN);
}

// Hàm khởi tạo DHT11
void DHT11_init(void)
{
    DHT11_set_pin_output();
    DHT11_write_pin(1); // Giữ pin ở mức cao
}

esp_err_t DHT11_read(dht11_data_t *data)
{
    uint8_t bits[5] = {0};
    uint8_t cnt = 7;
    uint8_t idx = 0;

    // MCU gửi tín hiệu bắt đầu tới DHT
    DHT11_set_pin_output();
    DHT11_write_pin(0);
    DHT11_delay_us(18000);
    DHT11_write_pin(1);
    DHT11_delay_us(30);
    DHT11_set_pin_input();

    if (DHT11_read_pin() == 1)
    {
        ESP_LOGE(TAG, "DHT11 not pulling data line low after start signal");
        return ESP_FAIL;
    }

    while (DHT11_read_pin() == 0)
        ;
    while (DHT11_read_pin() == 1)
        ;

    // DHT phản hồi lại MCU
    for (int i = 0; i < 40; i++)
    {
        while (DHT11_read_pin() == 0)
            ;
        uint32_t length = 0;
        while (DHT11_read_pin() == 1)
        {
            length++;
            DHT11_delay_us(1);
            if (length > 100)
            {
                ESP_LOGE(TAG, "DHT11 timing error");
                return ESP_FAIL;
            }
        }
        if (length > 30)
        {
            bits[idx] |= (1 << cnt);
        }
        if (cnt == 0)
        {
            cnt = 7;
            idx++;
        }
        else
        {
            cnt--;
        }
    }

    // Gán giá trị từ mảng nhận được vào cấu trúc dữ liệu
    data->humidity = bits[0];
    data->temperature = bits[2];

    uint8_t checksum = bits[0] + bits[1] + bits[2] + bits[3];
    if (bits[4] != checksum)
    {
        ESP_LOGE(TAG, "DHT11 checksum error");
        return ESP_FAIL;
    }

    return ESP_OK;
}
