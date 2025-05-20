#include "can_transmit_utils.h"
#include "can_config.h" // For temperature_queue and TEMP_CAN_ID
#include "driver/twai.h"
#include "esp_log.h"
#include <string.h> // For memcpy

static const char *TAG_CAN_TX = "CAN_TRANSMIT";

void can_transmit_task(void *pvParameters) {
    ESP_LOGI(TAG_CAN_TX, "CAN Transmit Task Started");
    float temperature_c;

    while (1) {
        // Wait for temperature data from the queue
        if (xQueueReceive(temperature_queue, &temperature_c, portMAX_DELAY) == pdPASS) {
            twai_message_t message;
            message.identifier = TEMP_CAN_ID;
            message.flags = TWAI_MSG_FLAG_NONE; // Or TWAI_MSG_FLAG_EXTD for extended ID
            message.data_length_code = sizeof(float); // Temperature is a float (4 bytes)
            
            // Copy temperature data to CAN message data field
            memcpy(message.data, &temperature_c, sizeof(float));

            // Ensure data bytes beyond the actual data length are zeroed or handled if DLC < 8
            for (int i = sizeof(float); i < TWAI_FRAME_MAX_DLC; i++) {
                message.data[i] = 0;
            }

            // Transmit CAN message
            esp_err_t ret = twai_transmit(&message, pdMS_TO_TICKS(1000)); // Wait 1 second max for TX
            if (ret == ESP_OK) {
                ESP_LOGI(TAG_CAN_TX, "Message transmitted: ID=0x%03lX, Temp=%.2f C", message.identifier, temperature_c);
            } else {
                ESP_LOGE(TAG_CAN_TX, "Failed to transmit message: %s", esp_err_to_name(ret));
                // Handle specific errors like TWAI_ERR_TX_QUEUE_FULL or TWAI_ERR_BUS_OFF
                if (ret == TWAI_ALERT_BUS_OFF) {
                    ESP_LOGW(TAG_CAN_TX, "CAN Bus is off, attempting recovery...");
                    // Attempt to recover bus-off state (optional, requires careful implementation)
                    // For simplicity, we just log here. Recovery might involve twai_initiate_recovery().
                }
            }
        }
        // Small delay to allow other tasks to run, especially if queue is empty for a while (though portMAX_DELAY handles blocking)
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}