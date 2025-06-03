#ifndef PERFORMANCE_COMMANDS_H
#define PERFORMANCE_COMMANDS_H

#include "../main/utils/CLI/cli_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register performance testing commands
 * Example of additional modular commands that can be easily added
 */
void register_performance_commands(void);

/**
 * @brief Performance command implementations
 */
int cmd_benchmark_cpu(int argc, char **argv);
int cmd_benchmark_memory(int argc, char **argv);
int cmd_stress_test(int argc, char **argv);
int cmd_profile_tasks(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif // PERFORMANCE_COMMANDS_H
