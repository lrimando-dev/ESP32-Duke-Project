#ifndef ADC_UTILS_H
#define ADC_UTILS_H

#include "esp_adc/adc_oneshot.h" // For adc_unit_t, adc_atten_t
#include "esp_adc/adc_cali.h"
#include <stdbool.h>

// ADC Configuration define used by calibration
#define ADC_CALI_BITWIDTH   ADC_BITWIDTH_DEFAULT // Default is 12-bit

/**
 * @brief Initialize ADC calibration scheme.
 *
 * @param unit ADC unit.
 * @param atten ADC attenuation.
 * @param out_cali_handle Pointer to store the calibration handle.
 * @return true if calibration was successful and can be used, false otherwise.
 */
bool initialize_adc_calibration(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_cali_handle);

/**
 * @brief Deinitialize ADC calibration scheme.
 *
 * @param cali_handle Calibration handle to deinitialize.
 */
void deinitialize_adc_calibration(adc_cali_handle_t cali_handle);

#endif // ADC_UTILS_H