#include <stdio.h> // For ESP_LOGI (indirectly)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "utils/CAN/can_config.h"         // For temperature_queue definition
#include "utils/CAN/can_driver_utils.h"   // For CAN driver initialization
#include "utils/CAN/can_receive_utils.h"  // For CAN receive task

static const char *TAG_MAIN = "APP_MAIN";

// All other functions, global variables, and specific ADC/LM35 defines have been moved.

void app_main(void)
{
    ESP_LOGI(TAG_MAIN, "ESP32 CAN Bus Receiver - Main App");

    // Initialize CAN bus driver
    if (can_driver_init() != ESP_OK) {
        ESP_LOGE(TAG_MAIN, "Failed to initialize CAN driver. Halting.");
        return; // Or handle error appropriately
    }
    ESP_LOGI(TAG_MAIN, "CAN driver initialized.");

    // You can initialize other things here if needed (like NVS for calibration data persistence)


    // Create the CAN receive task (optional, but useful for testing/general utility)
    xTaskCreate(can_receive_task,
                "can_receive_task",
                4096, // Stack size
                NULL, // Parameters
                5,    // Priority
                NULL); // Task handle
    ESP_LOGI(TAG_MAIN, "CAN receive task created.");

    ESP_LOGI(TAG_MAIN, "All tasks created. Application running.");
}