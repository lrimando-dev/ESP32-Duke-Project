#include "cli_interface.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include <stdarg.h>

static const char *TAG = "CLI_INTERFACE";

// CLI State
static bool cli_initialized = false;
static bool cli_running = false;
static TaskHandle_t cli_task_handle = NULL;
static SemaphoreHandle_t cli_mutex = NULL;
static cli_config_t current_config;

// ANSI Color codes for better output formatting
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Forward declarations
static void cli_task(void *pvParameters);
static int cmd_help(int argc, char **argv);
static int cmd_version(int argc, char **argv);
static int cmd_restart(int argc, char **argv);
static int cmd_free(int argc, char **argv);
static int cmd_heap(int argc, char **argv);
static int cmd_tasks(int argc, char **argv);

// Built-in commands
static const cli_command_t builtin_commands[] = {
    {
        .command = "help",
        .help = "Get help on commands. Usage: help [command]",
        .hint = NULL,
        .func = cmd_help,
        .argtable = NULL
    },
    {
        .command = "version",
        .help = "Show system version information",
        .hint = NULL,
        .func = cmd_version,
        .argtable = NULL
    },
    {
        .command = "restart",
        .help = "Restart the system",
        .hint = NULL,
        .func = cmd_restart,
        .argtable = NULL
    },
    {
        .command = "free",
        .help = "Show available heap memory",
        .hint = NULL,
        .func = cmd_free,
        .argtable = NULL
    },
    {
        .command = "heap",
        .help = "Show detailed heap information",
        .hint = NULL,
        .func = cmd_heap,
        .argtable = NULL
    },
    {
        .command = "tasks",
        .help = "Show FreeRTOS task information",
        .hint = NULL,
        .func = cmd_tasks,
        .argtable = NULL
    }
};

cli_config_t cli_get_default_config(void)
{
    cli_config_t config = {
        .echo_enabled = true,
        .history_enabled = true,
        .max_cmdline_length = CLI_MAX_CMDLINE_LENGTH,
        .history_save_path_len = 0,
        .history_save_path = NULL
    };
    return config;
}

cli_status_t cli_interface_init(cli_config_t *config)
{
    if (cli_initialized) {
        ESP_LOGW(TAG, "CLI already initialized");
        return CLI_STATUS_OK;
    }

    // Use default config if none provided
    if (config == NULL) {
        current_config = cli_get_default_config();
    } else {
        current_config = *config;
    }

    // Create mutex for thread safety
    cli_mutex = xSemaphoreCreateMutex();
    if (cli_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create CLI mutex");
        return CLI_STATUS_ERROR;
    }

    // Initialize NVS for history storage (if enabled)
    if (current_config.history_enabled) {
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        ESP_ERROR_CHECK(err);
    }    // Initialize console
    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = current_config.max_cmdline_length,
        .hint_color = 36  // Cyan color code
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    // Configure linenoise
    linenoiseSetMultiLine(1);
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);
    linenoiseHistorySetMaxLen(CLI_HISTORY_SIZE);
    linenoiseSetMaxLineLen(current_config.max_cmdline_length);

    // Configure UART for console - using new API
    uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);
    uart_vfs_dev_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    uart_vfs_dev_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);

    // Register built-in commands
    size_t builtin_count = sizeof(builtin_commands) / sizeof(builtin_commands[0]);
    for (size_t i = 0; i < builtin_count; i++) {
        cli_register_command(&builtin_commands[i]);
    }

    cli_initialized = true;
    ESP_LOGI(TAG, "CLI interface initialized successfully");

    return CLI_STATUS_OK;
}

cli_status_t cli_interface_start(void)
{
    if (!cli_initialized) {
        ESP_LOGE(TAG, "CLI not initialized");
        return CLI_STATUS_NOT_INITIALIZED;
    }

    if (cli_running) {
        ESP_LOGW(TAG, "CLI already running");
        return CLI_STATUS_OK;
    }

    // Register external commands if available
    if (cli_register_external_commands != NULL) {
        cli_register_external_commands();
    }

    // Create CLI task
    BaseType_t result = xTaskCreate(
        cli_task,
        "cli_task",
        CLI_TASK_STACK_SIZE,
        NULL,
        CLI_TASK_PRIORITY,
        &cli_task_handle
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create CLI task");
        return CLI_STATUS_ERROR;
    }

    cli_running = true;
    ESP_LOGI(TAG, "CLI interface started");

    return CLI_STATUS_OK;
}

cli_status_t cli_interface_stop(void)
{
    if (!cli_running) {
        ESP_LOGW(TAG, "CLI not running");
        return CLI_STATUS_OK;
    }

    if (cli_task_handle != NULL) {
        vTaskDelete(cli_task_handle);
        cli_task_handle = NULL;
    }

    cli_running = false;
    ESP_LOGI(TAG, "CLI interface stopped");

    return CLI_STATUS_OK;
}

cli_status_t cli_register_command(const cli_command_t *cmd)
{
    if (!cli_initialized) {
        ESP_LOGE(TAG, "CLI not initialized");
        return CLI_STATUS_NOT_INITIALIZED;
    }

    if (cmd == NULL || cmd->command == NULL || cmd->func == NULL) {
        ESP_LOGE(TAG, "Invalid command parameters");
        return CLI_STATUS_INVALID_ARG;
    }

    esp_console_cmd_t esp_cmd = {
        .command = cmd->command,
        .help = cmd->help,
        .hint = cmd->hint,
        .func = cmd->func,
        .argtable = cmd->argtable
    };

    esp_err_t err = esp_console_cmd_register(&esp_cmd);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register command '%s': %s", cmd->command, esp_err_to_name(err));
        return CLI_STATUS_ERROR;
    }

    ESP_LOGD(TAG, "Registered command: %s", cmd->command);
    return CLI_STATUS_OK;
}

cli_status_t cli_register_commands(const cli_command_t *commands, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        cli_status_t status = cli_register_command(&commands[i]);
        if (status != CLI_STATUS_OK) {
            ESP_LOGE(TAG, "Failed to register command %zu", i);
            return status;
        }
    }
    return CLI_STATUS_OK;
}

cli_status_t cli_unregister_command(const char *command)
{
    // ESP-IDF doesn't provide unregister functionality
    ESP_LOGW(TAG, "Command unregistration not supported by ESP-IDF");
    return CLI_STATUS_ERROR;
}

int cli_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

int cli_printf_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf(ANSI_COLOR_RED "[ERROR] " ANSI_COLOR_RESET);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

int cli_printf_success(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf(ANSI_COLOR_GREEN "[SUCCESS] " ANSI_COLOR_RESET);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

int cli_printf_warning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf(ANSI_COLOR_YELLOW "[WARNING] " ANSI_COLOR_RESET);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

bool cli_is_initialized(void)
{
    return cli_initialized;
}

bool cli_is_running(void)
{
    return cli_running;
}

// CLI Task implementation
static void cli_task(void *pvParameters)
{
    const char* prompt = CLI_PROMPT_STR;
    char* line;

    cli_printf(ANSI_COLOR_CYAN);
    cli_printf("===========================================\n");
    cli_printf("       ESP32 CLI Interface Ready\n");
    cli_printf("===========================================\n");
    cli_printf(ANSI_COLOR_RESET);
    cli_printf("Type 'help' to get the list of commands.\n");
    cli_printf("Use UP/DOWN arrows to navigate through command history.\n");
    cli_printf("Press TAB when typing command name to auto-complete.\n\n");

    while (cli_running) {
        line = linenoise(prompt);
        
        if (line == NULL) { // Break on EOF or error
            continue;
        }
        
        if (strlen(line) > 0) {
            linenoiseHistoryAdd(line);
            
            int ret;
            esp_err_t err = esp_console_run(line, &ret);
            if (err == ESP_ERR_NOT_FOUND) {
                cli_printf_error("Unrecognized command\n");
            } else if (err == ESP_ERR_INVALID_ARG) {
                // Command was recognized but had some error in arguments
            } else if (err == ESP_OK && ret != ESP_OK) {
                cli_printf_error("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
            } else if (err != ESP_OK) {
                cli_printf_error("Internal error: %s\n", esp_err_to_name(err));
            }
        }
        
        linenoiseFree(line);
    }

    ESP_LOGI(TAG, "CLI task ending");
    vTaskDelete(NULL);
}

// Built-in command implementations
static int cmd_help(int argc, char **argv)
{
    if (argc == 1) {
        cli_printf("\n" ANSI_COLOR_CYAN "Available commands:\n" ANSI_COLOR_RESET);
        cli_printf("Use 'help <command>' for detailed information about a specific command.\n");
        cli_printf("Type TAB for command completion, UP/DOWN arrows for history.\n\n");
        
        // List basic help since esp_console_cmd_get is not available
        cli_printf("Built-in commands:\n");
        cli_printf("  help      - Show this help message\n");
        cli_printf("  version   - Show system version information\n");
        cli_printf("  restart   - Restart the system\n");
        cli_printf("  free      - Show available heap memory\n");
        cli_printf("  heap      - Show detailed heap information\n");
        cli_printf("  tasks     - Show FreeRTOS task information\n");
    } else if (argc == 2) {
        // Simple help for individual commands
        if (strcmp(argv[1], "help") == 0) {
            cli_printf("help - Show help information\nUsage: help [command]\n");
        } else if (strcmp(argv[1], "version") == 0) {
            cli_printf("version - Show system version and chip information\n");
        } else if (strcmp(argv[1], "restart") == 0) {
            cli_printf("restart - Restart the ESP32 system\n");
        } else if (strcmp(argv[1], "free") == 0) {
            cli_printf("free - Show current heap memory usage\n");
        } else if (strcmp(argv[1], "heap") == 0) {
            cli_printf("heap - Show detailed heap memory statistics\n");
        } else if (strcmp(argv[1], "tasks") == 0) {
            cli_printf("tasks - Show FreeRTOS task information and statistics\n");
        } else {
            cli_printf_error("Command '%s' not found\n", argv[1]);
            return 1;
        }
    }
    return 0;
}

static int cmd_version(int argc, char **argv)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    uint32_t flash_size;
    esp_flash_get_size(NULL, &flash_size);
    
    cli_printf("\n" ANSI_COLOR_CYAN "System Information:\n" ANSI_COLOR_RESET);
    cli_printf("ESP-IDF Version: %s\n", esp_get_idf_version());
    cli_printf("Chip: %s\n", CONFIG_IDF_TARGET);
    cli_printf("Silicon revision: %d\n", chip_info.revision);    cli_printf("Cores: %d\n", chip_info.cores);
    cli_printf("Features: 0x%08X\n", (unsigned int)chip_info.features);
    cli_printf("Flash size: %ld MB\n", flash_size / (1024 * 1024));
    return 0;
}

static int cmd_restart(int argc, char **argv)
{
    cli_printf_warning("Restarting system...\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return 0;
}

static int cmd_free(int argc, char **argv)
{
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();
    
    cli_printf("\n" ANSI_COLOR_CYAN "Memory Information:\n" ANSI_COLOR_RESET);
    cli_printf("Free heap: %lu bytes\n", free_heap);
    cli_printf("Minimum free heap: %lu bytes\n", min_free_heap);
    cli_printf("Heap usage: %.1f%%\n", 
               (float)(min_free_heap) / (float)(free_heap + min_free_heap) * 100.0);
    return 0;
}

static int cmd_heap(int argc, char **argv)
{
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    
    cli_printf("\n" ANSI_COLOR_CYAN "Detailed Heap Information:\n" ANSI_COLOR_RESET);
    cli_printf("Total free bytes: %d\n", info.total_free_bytes);
    cli_printf("Total allocated bytes: %d\n", info.total_allocated_bytes);
    cli_printf("Largest free block: %d\n", info.largest_free_block);
    cli_printf("Minimum free bytes: %d\n", info.minimum_free_bytes);
    cli_printf("Allocated blocks: %d\n", info.allocated_blocks);
    cli_printf("Free blocks: %d\n", info.free_blocks);
    cli_printf("Total blocks: %d\n", info.total_blocks);
    return 0;
}

static int cmd_tasks(int argc, char **argv)
{
    cli_printf("\n" ANSI_COLOR_CYAN "FreeRTOS Task Information:\n" ANSI_COLOR_RESET);
    cli_printf("Task Name\t\tState\tPrio\tCore\tStack\n");
    cli_printf("=================================================\n");
    
    // Get task count
    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    cli_printf("Total Tasks: %u\n", task_count);
    
    // Get runtime stats if available
    cli_printf("Free Heap Size: %u bytes\n", (unsigned int)esp_get_free_heap_size());
    cli_printf("Minimum Free Heap: %u bytes\n", (unsigned int)esp_get_minimum_free_heap_size());
    
    // Note: Individual task details require configUSE_TRACE_FACILITY to be enabled
    cli_printf("\nNote: Enable CONFIG_FREERTOS_USE_TRACE_FACILITY for detailed task info\n");
    
    return 0;
}