#include "adc_utils.h"
#include "esp_log.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG_ADC_UTILS = "ADC_CALI";

bool initialize_adc_calibration(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_cali_handle) {
    *out_cali_handle = NULL;
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_CALI_BITWIDTH,
    };
    esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, out_cali_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG_ADC_UTILS, "Calibration Success");
        return true;
    } else if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG_ADC_UTILS, "Calibration scheme not supported, using raw ADC values");
    } else {
        ESP_LOGE(TAG_ADC_UTILS, "Calibration failed: %s", esp_err_to_name(ret));
    }
    return false;
}

void deinitialize_adc_calibration(adc_cali_handle_t cali_handle) {
    if (cali_handle) {
        ESP_LOGI(TAG_ADC_UTILS, "Deregistering ADC calibration scheme");
        ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(cali_handle));
    }
}