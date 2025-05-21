#include <stdio.h> // For ESP_LOGI (indirectly)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "utils/CAN/can_config.h"
#include "utils/CAN/can_driver_utils.h"
#include "utils/CAN/can_receive_utils.h"

static const char *TAG_MAIN = "APP_MAIN";

void app_main(void)
{
    ESP_LOGI(TAG_MAIN, "ESP32 CAN Bus Receiver - Main App");

    // Initialize CAN bus driver
    if (can_driver_init() != ESP_OK) {
        ESP_LOGE(TAG_MAIN, "Failed to initialize CAN driver. Halting.");
        return;
    }
    ESP_LOGI(TAG_MAIN, "CAN driver initialized.");


    xTaskCreate(can_receive_task, "can_receive_task", 4096, NULL, 5, NULL); 
    ESP_LOGI(TAG_MAIN, "CAN receive task created.");

    ESP_LOGI(TAG_MAIN, "All tasks created. Application running.");
}