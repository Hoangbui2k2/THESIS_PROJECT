#include "dimmer.h"
#include "potentiometer.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_rom_sys.h" // Include header for esp_rom_delay_us

static const char *TAG = "DIMMER";

// Phase delay in microseconds
volatile int phase_delay = 5000; // Default value, will be adjusted by potentiometer

// Timer handle for TRIAC pulse
static esp_timer_handle_t triac_timer;

// Timer callback for TRIAC pulse
void triac_pulse_callback(void* arg) {
    gpio_set_level(GPIO_TRIAC_CONTROL, 1);
    esp_rom_delay_us(10);  // TRIAC pulse width, adjust if needed
    gpio_set_level(GPIO_TRIAC_CONTROL, 0);
}

// Initialize dimmer
void dimmer_init(void) {
    // Configure GPIO for zero-cross detection
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_ZERO_CROSS),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf);

    // Configure GPIO for TRIAC control
    gpio_set_direction(GPIO_TRIAC_CONTROL, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_TRIAC_CONTROL, 0);

    // Initialize potentiometer (moved to potentiometer.c)
    potentiometer_init();

    // Initialize and start timer for TRIAC pulse
    esp_timer_create_args_t timer_args = {
        .callback = &triac_pulse_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "triac_pulse"
    };
    esp_timer_create(&timer_args, &triac_timer);

    // Install ISR service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_ZERO_CROSS, zero_cross_isr_handler, NULL);

    ESP_LOGI(TAG, "Dimmer initialized");
}

// Set phase delay (microseconds)
void dimmer_set_phase_delay(int delay) {
    phase_delay = delay;
}

// ISR handler for zero-cross detection
void IRAM_ATTR zero_cross_isr_handler(void* arg) {
    // Schedule TRIAC pulse after phase delay
    esp_timer_start_once(triac_timer, phase_delay);
}
