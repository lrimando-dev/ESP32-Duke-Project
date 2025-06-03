#include "utils/CLI/cli_interface.h"
#include "test_commands.h"
#include "performance_commands.h"

/**
 * @brief External command registration implementation
 * This function is called by the CLI interface to register external commands
 * It acts as a bridge between the CLI system and external test cases
 */
void cli_register_external_commands(void)
{
    // Register test case commands
    register_test_commands();
    
    // Register performance testing commands
    register_performance_commands();
    
    // Add any other external command registrations here
    // For example, if you have other modules in TestCases:
    // register_calibration_commands();
    // register_debug_commands();
    // register_protocol_commands();
}
