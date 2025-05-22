#include "can_driver_utils.h"
#include "can_config.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG_CAN_DRIVER = "CAN_DRIVER";

esp_err_t can_driver_init(void) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO, CAN_RX_GPIO, TWAI_MODE_NORMAL);
    g_config.tx_queue_len = CAN_TX_QUEUE_LENGTH;
    g_config.rx_queue_len = CAN_RX_QUEUE_LENGTH;

    twai_timing_config_t t_config = CAN_TIMIMG;

    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_CAN_DRIVER, "Failed to install TWAI driver: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG_CAN_DRIVER, "TWAI driver installed");


    ret = twai_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_CAN_DRIVER, "Failed to start TWAI driver: %s", esp_err_to_name(ret));
        twai_driver_uninstall();
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