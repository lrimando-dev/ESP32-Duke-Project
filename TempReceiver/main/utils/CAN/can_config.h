#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H

#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


#define CAN_TX_GPIO         GPIO_NUM_21
#define CAN_RX_GPIO         GPIO_NUM_22

#define TEMP_CAN_ID         0x1A0

#endif