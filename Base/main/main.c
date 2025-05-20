#include <stdio.h> // For ESP_LOGI (indirectly)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "utils/TempSensor/temp_sensor.h" // Include the new header for the LM35 task
#include "utils/CAN/can_config.h"         // For temperature_queue definition
#include "utils/CAN/can_driver_utils.h"   // For CAN driver initialization
#include "utils/CAN/can_transmit_utils.h" // For CAN transmit task
#include "utils/CAN/can_receive_utils.h"  // For CAN receive task

static const char *TAG_MAIN = "APP_MAIN";

// All other functions, global variables, and specific ADC/LM35 defines have been moved.

// Define the temperature queue (declared extern in can_config.h)
QueueHandle_t temperature_queue = NULL;

void app_main(void)
{
    ESP_LOGI(TAG_MAIN, "ESP32 LM35 Temperature Sensor with CAN - Main App");

    // Create the temperature data queue
    // Queue for 10 float values. Adjust size as needed.
    temperature_queue = xQueueCreate(10, sizeof(float)); 
    if (temperature_queue == NULL) {
        ESP_LOGE(TAG_MAIN, "Failed to create temperature queue. Halting.");
        return; // Or handle error appropriately
    }
    ESP_LOGI(TAG_MAIN, "Temperature queue created.");

    // Initialize CAN bus driver
    if (can_driver_init() != ESP_OK) {
        ESP_LOGE(TAG_MAIN, "Failed to initialize CAN driver. Halting.");
        // Optionally deallocate queue if created: vQueueDelete(temperature_queue);
        return; // Or handle error appropriately
    }
    ESP_LOGI(TAG_MAIN, "CAN driver initialized.");

    // You can initialize other things here if needed (like NVS for calibration data persistence)

    // Create the temperature reader task
    xTaskCreate(lm35_reader_task,    // Task function from lm35_sensor.h
                "lm35_reader_task",  // Name of the task
                4096,                // Stack size in words
                NULL,                // Task input parameter
                5,                   // Priority of the task
                NULL);               // Task handle
    ESP_LOGI(TAG_MAIN, "LM35 reader task created.");

    // Create the CAN transmit task
    xTaskCreate(can_transmit_task,
                "can_transmit_task",
                4096, // Stack size
                NULL, // Parameters
                5,    // Priority
                NULL); // Task handle
    ESP_LOGI(TAG_MAIN, "CAN transmit task created.");

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