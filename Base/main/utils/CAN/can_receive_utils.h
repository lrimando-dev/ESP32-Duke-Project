#ifndef CAN_RECEIVE_UTILS_H
#define CAN_RECEIVE_UTILS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Task to receive and process CAN messages.
 *
 * @param pvParameters Task parameters (not used).
 */
void can_receive_task(void *pvParameters);

#endif // CAN_RECEIVE_UTILS_H