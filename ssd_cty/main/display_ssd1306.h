/*
* @Author: nhantt
* @Date:   2020-02-02
* @Last Modified by:   nhantt
* @Last Modified time: 2022-01-06
*/

#ifndef __DISPLAY_OLED_H__
#define __DISPLAY_OLED_H__

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>

#include "u8g2_esp32_hal.h"

extern u8g2_t u8g2;

#define LCD_WIDTH	128
#define LCD_HEIGHT	64

void Display_Init(void);
void Display_Task(void *argv);

#endif