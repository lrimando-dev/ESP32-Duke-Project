#ifndef CLI_INTERFACE_H
#define CLI_INTERFACE_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "esp_system.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "driver/uart_vfs.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

#ifdef __cplusplus
extern "C" {
#endif

// CLI Configuration
#define CLI_PROMPT_STR "ESP32-CLI> "
#define CLI_MAX_CMDLINE_LENGTH 256
#define CLI_TASK_STACK_SIZE 4096
#define CLI_TASK_PRIORITY 5
#define CLI_HISTORY_SIZE 30

// Command registration callback type
typedef int (*cli_command_func_t)(int argc, char **argv);

// Command structure for external registration
typedef struct {
    const char *command;
    const char *help;
    const char *hint;
    cli_command_func_t func;
    void *argtable;
} cli_command_t;

// CLI Status
typedef enum {
    CLI_STATUS_OK = 0,
    CLI_STATUS_ERROR = -1,
    CLI_STATUS_INVALID_ARG = -2,
    CLI_STATUS_NOT_INITIALIZED = -3
} cli_status_t;

// CLI Configuration structure
typedef struct {
    bool echo_enabled;
    bool history_enabled;
    uint32_t max_cmdline_length;
    uint32_t history_save_path_len;
    char *history_save_path;
} cli_config_t;

/**
 * @brief Initialize the CLI interface
 * 
 * @param config CLI configuration structure (can be NULL for defaults)
 * @return cli_status_t CLI_STATUS_OK on success
 */
cli_status_t cli_interface_init(cli_config_t *config);

/**
 * @brief Start the CLI task
 * 
 * @return cli_status_t CLI_STATUS_OK on success
 */
cli_status_t cli_interface_start(void);

/**
 * @brief Stop the CLI task
 * 
 * @return cli_status_t CLI_STATUS_OK on success
 */
cli_status_t cli_interface_stop(void);

/**
 * @brief Register a command with the CLI
 * 
 * @param cmd Command structure containing command info and handler
 * @return cli_status_t CLI_STATUS_OK on success
 */
cli_status_t cli_register_command(const cli_command_t *cmd);

/**
 * @brief Register multiple commands at once
 * 
 * @param commands Array of command structures
 * @param count Number of commands in the array
 * @return cli_status_t CLI_STATUS_OK on success
 */
cli_status_t cli_register_commands(const cli_command_t *commands, size_t count);

/**
 * @brief Unregister a command from the CLI
 * 
 * @param command Command name to unregister
 * @return cli_status_t CLI_STATUS_OK on success
 */
cli_status_t cli_unregister_command(const char *command);

/**
 * @brief Print formatted output to CLI console
 * 
 * @param format Printf-style format string
 * @param ... Variable arguments
 * @return int Number of characters printed
 */
int cli_printf(const char *format, ...);

/**
 * @brief Print error message to CLI console
 * 
 * @param format Printf-style format string
 * @param ... Variable arguments
 * @return int Number of characters printed
 */
int cli_printf_error(const char *format, ...);

/**
 * @brief Print success message to CLI console
 * 
 * @param format Printf-style format string
 * @param ... Variable arguments
 * @return int Number of characters printed
 */
int cli_printf_success(const char *format, ...);

/**
 * @brief Print warning message to CLI console
 * 
 * @param format Printf-style format string
 * @param ... Variable arguments
 * @return int Number of characters printed
 */
int cli_printf_warning(const char *format, ...);

/**
 * @brief Get CLI initialization status
 * 
 * @return true if CLI is initialized, false otherwise
 */
bool cli_is_initialized(void);

/**
 * @brief Get CLI running status
 * 
 * @return true if CLI task is running, false otherwise
 */
bool cli_is_running(void);

/**
 * @brief Get default CLI configuration
 * 
 * @return cli_config_t Default configuration structure
 */
cli_config_t cli_get_default_config(void);

/**
 * @brief External command registration function (to be implemented by users)
 * This function should be implemented in TestCases or other external modules
 * to register their specific commands
 */
extern void cli_register_external_commands(void) __attribute__((weak));

#ifdef __cplusplus
}
#endif

#endif // CLI_INTERFACE_H