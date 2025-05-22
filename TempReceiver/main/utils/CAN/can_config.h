#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H

#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


#define CAN_TX_GPIO         GPIO_NUM_21
#define CAN_RX_GPIO         GPIO_NUM_22

#define TEMP_CAN_ID         0x515

#define CAN_TX_QUEUE_LENGTH  5
#define CAN_RX_QUEUE_LENGTH  5

// #define CAN_TIMIMG          TWAI_TIMING_CONFIG_125KBITS()
// #define CAN_TIMIMG          TWAI_TIMING_CONFIG_250KBITS()
#define CAN_TIMIMG          TWAI_TIMING_CONFIG_500KBITS()
// #define CAN_TIMIMG          TWAI_TIMING_CONFIG_1MBITS()
#endif