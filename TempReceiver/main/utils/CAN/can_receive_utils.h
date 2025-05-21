#ifndef CAN_RECEIVE_UTILS_H
#define CAN_RECEIVE_UTILS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void can_receive_task(void *pvParameters);

#endif