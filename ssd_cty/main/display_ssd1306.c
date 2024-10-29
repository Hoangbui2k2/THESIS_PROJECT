/*
* @Author: nhantt
* @Date:   2020-02-02
* @Last Modified by:   nhantt
* @Last Modified time: 2022-01-06
*/

#include "display_ssd1306.h"
#include "ph_logo.h"
#include "thermostat_ui.h"
#include "sensor.h"
#include "ip5306.h"
#include "main.h"
// #include "ble_gatt_server.h"
u8g2_t u8g2;
uint32_t lcdRefesh;
static uint8_t *buf;
bool status_connect_ble = false;

static const char *TAG = "OLED";

void Display_Init(void)
{
    // initialize the u8g2 hal
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.sda = GPIO_NUM_1;
    u8g2_esp32_hal.scl = GPIO_NUM_2;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    // initialize the u8g2 library
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R2, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);
    u8g2_InitDisplay(&u8g2);
    // // u8g2_InitDisplay(&ui_screen);
    // u8g2_SetPowerSave(&u8g2, 0); // wake up display
    // u8g2_ClearDisplay(&u8g2);
    // u8g2_ClearBuffer(&u8g2);
    // wake up the display
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_SetContrast(&u8g2, 100);
    u8g2_ClearBuffer(&u8g2);
    ESP_LOGI(TAG, "u8g2_ClearBuffer");

    Logo_screen();

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    thermostat_init(0, 1, status_connect_ble);

}

void Display_Task(void *argv)
{
  Display_Init();
  while(1)
  {
    thermostat_manager(temp_logger_sensors[Temp_sensor].data, flash_value.sensors[Temp_sensor].Unit, status_connect_ble);
    u8g2_SendBuffer(&u8g2);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}



