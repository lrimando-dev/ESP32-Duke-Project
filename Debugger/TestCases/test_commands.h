#ifndef TEST_COMMANDS_H
#define TEST_COMMANDS_H

#include "../main/utils/CLI/cli_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register test case commands with the CLI
 * This function is called automatically by the CLI interface
 * when cli_register_external_commands is invoked
 */
void register_test_commands(void);

/**
 * @brief Test command implementations
 */
int cmd_test_led(int argc, char **argv);
int cmd_test_sensors(int argc, char **argv);
int cmd_test_communication(int argc, char **argv);
int cmd_test_memory(int argc, char **argv);
int cmd_run_all_tests(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif // TEST_COMMANDS_H
