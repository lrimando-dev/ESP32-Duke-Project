#include "can_receive_utils.h"
#include "can_config.h" // For potential shared configs if needed in future
#include "driver/twai.h"
#include "esp_log.h"

#include <string.h>

static const char *TAG_CAN_RX = "CAN_RECEIVE";

void can_receive_task(void *pvParameters) {
    ESP_LOGI(TAG_CAN_RX, "CAN Receive Task Started");
    twai_message_t rx_message;

    while (1) {
        // Wait to receive a message
        esp_err_t ret = twai_receive(&rx_message, pdMS_TO_TICKS(portMAX_DELAY)); // Block indefinitely

        if (ret == ESP_OK) {
            ESP_LOGI(TAG_CAN_RX, "Message received: ID=0x%03lX, DLC=%d", rx_message.identifier, rx_message.data_length_code);
            // Log data bytes
            char data_str[3 * TWAI_FRAME_MAX_DLC + 1] = {0};
            for (int i = 0; i < rx_message.data_length_code; i++) {
                sprintf(data_str + i * 3, "%02X ", rx_message.data[i]);
            }
            ESP_LOGI(TAG_CAN_RX, "Data: %s", data_str);

            // Example: Check if it's a temperature message (though this task is generic)
            if (rx_message.identifier == TEMP_CAN_ID && rx_message.data_length_code == sizeof(float)) {
                float received_temp;
                memcpy(&received_temp, rx_message.data, sizeof(float));
                ESP_LOGI(TAG_CAN_RX, "Received temperature: %.2f C", received_temp);
            }

        } else if (ret == ESP_ERR_TIMEOUT) {
            // Timeout, no message received (should not happen with portMAX_DELAY unless driver is stopped)
            // Continue loop
        } else {
            ESP_LOGE(TAG_CAN_RX, "Failed to receive message: %s", esp_err_to_name(ret));
            // If bus-off or other critical error, may need to re-init or handle
            if (ret == TWAI_ALERT_BUS_RECOVERED || ret == TWAI_ALERT_BUS_OFF) {
                ESP_LOGW(TAG_CAN_RX, "CAN bus issue detected: %s. May need re-initialization.", esp_err_to_name(ret));
                // Potentially signal main task or attempt recovery
                vTaskDelay(pdMS_TO_TICKS(1000)); // Wait before retrying
            }
        }
        // Small delay to yield, though twai_receive with timeout handles blocking
         vTaskDelay(pdMS_TO_TICKS(10));
    }
}