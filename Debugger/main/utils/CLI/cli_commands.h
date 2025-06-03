#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

#include "cli_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register utility-specific CLI commands
 * This function registers commands that interact with the project's utility modules
 */
void cli_register_utility_commands(void);

/**
 * @brief ADC utility commands
 */
int cmd_adc_read(int argc, char **argv);
int cmd_adc_calibrate(int argc, char **argv);

/**
 * @brief CAN utility commands
 */
int cmd_can_send(int argc, char **argv);
int cmd_can_receive(int argc, char **argv);
int cmd_can_status(int argc, char **argv);

/**
 * @brief Temperature sensor commands
 */
int cmd_temp_read(int argc, char **argv);

/**
 * @brief AD5693 DAC commands
 */
int cmd_dac_set(int argc, char **argv);
int cmd_dac_read(int argc, char **argv);

/**
 * @brief System diagnostic commands
 */
int cmd_gpio_set(int argc, char **argv);
int cmd_gpio_get(int argc, char **argv);
int cmd_i2c_scan(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif // CLI_COMMANDS_H