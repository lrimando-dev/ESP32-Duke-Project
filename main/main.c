#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "LM35_TEMP";

// ADC Configuration
#define LM35_ADC_UNIT       ADC_UNIT_1
#define LM35_ADC_CHANNEL    ADC_CHANNEL_6 // GPIO34 is ADC1_CH6
#define LM35_ADC_ATTEN      ADC_ATTEN_DB_11 // For full 0-3.3V range approx. (ESP32 Vref ~1.1V, 11dB atten gives ~3.1-3.3V full scale)
#define LM35_ADC_BITWIDTH   ADC_BITWIDTH_DEFAULT // Default is 12-bit

// Calibration handle
static adc_cali_handle_t cali_handle = NULL;
static bool do_calibration = false;

// ADC Oneshot unit handle
static adc_oneshot_unit_handle_t adc_handle;


// Function to initialize ADC calibration
static bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Should match your ADC read bitwidth
    };
    esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, out_handle);
    if (ret == ESP_OK) {
        do_calibration = true;
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, using raw ADC values");
    } else {
        ESP_LOGE(TAG, "Calibration failed: %s", esp_err_to_name(ret));
    }
    return (ret == ESP_OK);
}

// Function to deinitialize ADC calibration (if needed)
static void adc_calibration_deinit(adc_cali_handle_t handle)
{
    if (handle) {
        ESP_LOGI(TAG, "Deregistering ADC calibration scheme");
        ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
    }
}


void lm35_reader_task(void *pvParameters)
{
    // ADC Oneshot Init
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = LM35_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE, // ULP mode not used
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    // ADC Channel Config
    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = LM35_ADC_BITWIDTH,
        .atten = LM35_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, LM35_ADC_CHANNEL, &channel_config));

    // ADC Calibration (try to initialize)
    adc_calibration_init(LM35_ADC_UNIT, LM35_ADC_ATTEN, &cali_handle);


    ESP_LOGI(TAG, "LM35 Reader Task Started. Reading from ADC1_CH%d (GPIO34)", LM35_ADC_CHANNEL);

    while (1) {
        int adc_raw_reading;
        int voltage_mv;

        // Read ADC
        esp_err_t read_err = adc_oneshot_read(adc_handle, LM35_ADC_CHANNEL, &adc_raw_reading);
        if (read_err == ESP_OK) {
            // ESP_LOGI(TAG, "Raw ADC Reading: %d", adc_raw_reading); // For debugging

            if (do_calibration && cali_handle) {
                read_err = adc_cali_raw_to_voltage(cali_handle, adc_raw_reading, &voltage_mv);
                if (read_err != ESP_OK) {
                    ESP_LOGW(TAG, "Calibration to voltage failed, using approximation.");
                    // Fallback or simpler calculation if calibration fails at runtime for some reason
                    // This is a rough approximation if V_REF for 11dB is ~3100mV
                    voltage_mv = (adc_raw_reading * 3100) / 4095;
                }
            } else {
                // Simplified conversion if no calibration
                // This depends heavily on the actual Vref and attenuation characteristics.
                // Assuming 11dB attenuation gives a full scale of ~3100mV for a 12-bit ADC (0-4095)
                // This is less accurate than calibrated values.
                voltage_mv = (adc_raw_reading * 3100) / 4095; // Adjust 3100 based on your ESP32's characteristics or datasheet
            }

            // LM35 gives 10mV per degree Celsius
            float temperature_c = (float)voltage_mv / 10.0;

            ESP_LOGI(TAG, "Voltage: %d mV, Temperature: %.2f C", voltage_mv, temperature_c);

        } else {
            ESP_LOGE(TAG, "ADC Read Error: %s", esp_err_to_name(read_err));
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Read every 2 seconds
    }

    // Cleanup (if task were to exit, which it doesn't here)
    adc_oneshot_del_unit(adc_handle);
    if (do_calibration) {
        adc_calibration_deinit(cali_handle);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 LM35 Temperature Sensor Example");

    // You can initialize other things here if needed (like NVS for calibration data persistence)

    // Create the temperature reader task
    xTaskCreate(lm35_reader_task,    // Task function
                "lm35_reader_task",  // Name of the task
                4096,                // Stack size in words
                NULL,                // Task input parameter
                5,                   // Priority of the task
                NULL);               // Task handle
}