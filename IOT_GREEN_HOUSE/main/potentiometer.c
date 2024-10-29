#include "potentiometer.h"
#include "esp_log.h"
#include "driver/adc.h"

static const char *TAG = "POTENTIOMETER";

// Initialize potentiometer ADC
void potentiometer_init(void) {
    // Configure ADC for potentiometer reading
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_POTENTIOMETER, ADC_ATTEN_DB_11);
    ESP_LOGI(TAG, "Potentiometer initialized");
}

// Update phase delay based on potentiometer reading
void potentiometer_update(void) {
    // Read potentiometer value
    int pot_value = adc1_get_raw(ADC_POTENTIOMETER);
    // Convert ADC value to phase delay
    phase_delay = 3500 + (pot_value * 6000 / 4095); // Map potentiometer to 1000-8000 microseconds
    ESP_LOGI(TAG, "Potentiometer value: %d, Phase delay: %d", pot_value, phase_delay);
}
