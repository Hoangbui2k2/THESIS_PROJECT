#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_rom_sys.h" // Include header for esp_rom_delay_us

#define GPIO_ZERO_CROSS 14
#define GPIO_TRIAC_CONTROL 27
#define ADC_POTENTIOMETER ADC1_CHANNEL_4

static const char *TAG = "DIMMER";

// Phase delay in microseconds
volatile int phase_delay = 5000; // Default value, will be adjusted by potentiometer

// Timer callback for TRIAC pulse
void triac_pulse_callback(void* arg) {
    gpio_set_level(GPIO_TRIAC_CONTROL, 1);
    esp_rom_delay_us(10);  // TRIAC pulse width, adjust if needed
    gpio_set_level(GPIO_TRIAC_CONTROL, 0);
}

void IRAM_ATTR zero_cross_isr_handler(void* arg) {
    // Schedule TRIAC pulse
    esp_timer_handle_t triac_timer;
    esp_timer_create_args_t timer_args = {
        .callback = &triac_pulse_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "triac_pulse"
    };

    esp_timer_create(&timer_args, &triac_timer);
    esp_timer_start_once(triac_timer, phase_delay);
}

void read_potentiometer_task(void *arg) {
    while (1) {
        // Read potentiometer value
        int pot_value = adc1_get_raw(ADC_POTENTIOMETER);
        // Convert ADC value to phase delay
        phase_delay = 1000 + (pot_value * 9000 / 4095); // Map potentiometer to 1000-8000 microseconds
        ESP_LOGI(TAG, "Potentiometer value: %d, Phase delay: %d", pot_value, phase_delay);
        vTaskDelay(pdMS_TO_TICKS(100)); // Adjust reading frequency as needed
    }
}

void app_main(void) {
    // Configure GPIO for zero-cross detection
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_ZERO_CROSS);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    // Configure GPIO for TRIAC control
    gpio_set_direction(GPIO_TRIAC_CONTROL, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_TRIAC_CONTROL, 0);

    // Configure ADC for potentiometer reading
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_POTENTIOMETER, ADC_ATTEN_DB_11);

    // Install ISR service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_ZERO_CROSS, zero_cross_isr_handler, NULL);

    // Create task to read potentiometer
    xTaskCreate(read_potentiometer_task, "read_potentiometer_task", 2048, NULL, 5, NULL);

    ESP_LOGI(TAG, "Dimmer initialized");
}
