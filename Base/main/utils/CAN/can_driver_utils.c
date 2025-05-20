#include "can_driver_utils.h"
#include "can_config.h"
#include "esp_log.h"
#include "driver/gpio.h" // Required for TWAI_GENERAL_CONFIG_DEFAULT

static const char *TAG_CAN_DRIVER = "CAN_DRIVER";

esp_err_t can_driver_init(void) {
    // Initialize TWAI general configuration
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO, CAN_RX_GPIO, TWAI_MODE_NORMAL);
    g_config.tx_queue_len = 5; // Number of messages TX queue can hold
    g_config.rx_queue_len = 5; // Number of messages RX queue can hold

    // Initialize TWAI timing configuration
    // Common speeds: TWAI_TIMING_CONFIG_125KBITS(), TWAI_TIMING_CONFIG_250KBITS(), TWAI_TIMING_CONFIG_500KBITS(), TWAI_TIMING_CONFIG_1MBITS()
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

    // Initialize TWAI filter configuration (accept all messages)
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install TWAI driver
    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_CAN_DRIVER, "Failed to install TWAI driver: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG_CAN_DRIVER, "TWAI driver installed");

    // Start TWAI driver
    ret = twai_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_CAN_DRIVER, "Failed to start TWAI driver: %s", esp_err_to_name(ret));
        twai_driver_uninstall(); // Clean up if start fails
        return ret;
    }
    ESP_LOGI(TAG_CAN_DRIVER, "TWAI driver started");
    return ESP_OK;
}

void can_driver_deinit(void) {
    esp_err_t ret = twai_stop();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_CAN_DRIVER, "Failed to stop TWAI driver: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG_CAN_DRIVER, "TWAI driver stopped");
    }

    ret = twai_driver_uninstall();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_CAN_DRIVER, "Failed to uninstall TWAI driver: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG_CAN_DRIVER, "TWAI driver uninstalled");
    }
}