#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H

#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Define CAN GPIOs - ESP32 default: TX GPIO21, RX GPIO22. Adjust if needed.
#define CAN_TX_GPIO         GPIO_NUM_21
#define CAN_RX_GPIO         GPIO_NUM_22

// Define a CAN ID for temperature messages
#define TEMP_CAN_ID         0x1A0 // Example CAN ID

#endif // CAN_CONFIG_H