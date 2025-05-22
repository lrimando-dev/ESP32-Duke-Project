#include "can_transmit_utils.h"
#include "can_driver_utils.h"
#include "can_config.h"
#include "driver/twai.h"
#include "esp_log.h"
#include <string.h>
static const char *TAG_CAN_TX = "CAN_TRANSMIT";

void can_transmit_task(void *pvParameters) {
    ESP_LOGI(TAG_CAN_TX, "CAN Transmit Task Started");
    float temperature_c;

    while (1) {
        if (xQueueReceive(temperature_queue, &temperature_c, portMAX_DELAY) == pdPASS) {
            twai_message_t message;
            message.identifier = TEMP_CAN_ID;
            message.flags = TWAI_MSG_FLAG_NONE;
            message.data_length_code = sizeof(float);
            
            memcpy(message.data, &temperature_c, sizeof(float));

            for (int i = sizeof(float); i < TWAI_FRAME_MAX_DLC; i++) {
                message.data[i] = 0;
            }

            esp_err_t espStatus = twai_transmit(&message, pdMS_TO_TICKS(1000));
            if (espStatus == ESP_OK) {
                ESP_LOGI(TAG_CAN_TX, "Message transmitted: ID=0x%03lX, Temp=%.2f C", message.identifier, temperature_c); 
            }else {
                ESP_LOGE(TAG_CAN_TX, "Failed to transmit message: %s", esp_err_to_name(espStatus));
                if (espStatus == TWAI_ALERT_BUS_OFF) {
                    ESP_LOGW(TAG_CAN_TX, "CAN Bus is off");
                } else if (espStatus == ESP_ERR_INVALID_STATE){
                    ESP_LOGW(TAG_CAN_TX, "Invalid state error, attempting to reinitialize CAN driver");
                    // handle invalid state error, restart CAN driver
                    can_driver_deinit();
                    vTaskDelay(pdMS_TO_TICKS(100)); 
                    if (can_driver_init() == ESP_OK) {
                        ESP_LOGI(TAG_CAN_TX, "CAN driver reinitialized, retrying transmission");
                    } else {
                        ESP_LOGE(TAG_CAN_TX, "Failed to reinitialize CAN driver");
                    }
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}