#include "temp_sensor.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h" // For adc_cali_raw_to_voltage
#include "adc_utils.h"       // For ADC calibration functions
#include "utils/CAN/can_config.h" // For temperature_queue extern declaration

static const char *TAG_LM35 = "LM35_TASK";

// ADC Configuration specific to LM35
#define LM35_ADC_UNIT       ADC_UNIT_1
#define LM35_ADC_CHANNEL    ADC_CHANNEL_6 // GPIO34 is ADC1_CH6
#define LM35_ADC_ATTEN      ADC_ATTEN_DB_12
#define LM35_ADC_BITWIDTH   ADC_BITWIDTH_DEFAULT

// Static variables for this module
static adc_cali_handle_t cali_handle = NULL;
static bool do_calibration = false;
static adc_oneshot_unit_handle_t adc_handle;


void lm35_reader_task(void *pvParameters) {
    // ADC Oneshot Init
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = LM35_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    // ADC Channel Config
    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = LM35_ADC_BITWIDTH,
        .atten = LM35_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, LM35_ADC_CHANNEL, &channel_config));

    // ADC Calibration
    // The initialize_adc_calibration function now returns whether calibration is active
    // and sets the cali_handle.
    do_calibration = initialize_adc_calibration(LM35_ADC_UNIT, LM35_ADC_ATTEN, &cali_handle);

    ESP_LOGI(TAG_LM35, "LM35 Reader Task Started. Reading from ADC1_CH%d (GPIO34)", LM35_ADC_CHANNEL);

    while (1) {
        int adc_raw_reading;
        int voltage_mv;

        esp_err_t read_err = adc_oneshot_read(adc_handle, LM35_ADC_CHANNEL, &adc_raw_reading);
        if (read_err == ESP_OK) {
            if (do_calibration && cali_handle) {
                read_err = adc_cali_raw_to_voltage(cali_handle, adc_raw_reading, &voltage_mv);
                if (read_err != ESP_OK) {
                    ESP_LOGW(TAG_LM35, "Calibration to voltage failed, using approximation.");
                    // Fallback: Approx. for 11dB attenuation, 12-bit ADC, Vref ~3.1V
                    voltage_mv = (adc_raw_reading * 3100) / 4095;
                }
            } else {
                // Simplified conversion if no calibration or if calibration failed during init
                // Approx. for 11dB attenuation, 12-bit ADC, Vref ~3.1V
                voltage_mv = (adc_raw_reading * 3100) / 4095;
            }

            // LM35 gives 10mV per degree Celsius
            float temperature_c = (float)voltage_mv / 10.0;
            ESP_LOGI(TAG_LM35, "Voltage: %d mV, Temperature: %.2f C", voltage_mv, temperature_c);

            
            // Send temperature to CAN queue
            if (temperature_queue != NULL) {
                if (xQueueSend(temperature_queue, &temperature_c, pdMS_TO_TICKS(100)) != pdPASS) {
                    ESP_LOGE(TAG_LM35, "Failed to send temperature to queue");
                }
            } else {
                ESP_LOGE(TAG_LM35, "Temperature queue not initialized!");
            }


        } else {
            ESP_LOGE(TAG_LM35, "ADC Read Error: %s", esp_err_to_name(read_err));
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // Read every 2 seconds
    }

    // Cleanup (if task were to exit, which it doesn't in this loop)
    adc_oneshot_del_unit(adc_handle);
    if (cali_handle) { // Check cali_handle directly as do_calibration might be true even if handle is later invalidated
        deinitialize_adc_calibration(cali_handle);
    }
    vTaskDelete(NULL);
}