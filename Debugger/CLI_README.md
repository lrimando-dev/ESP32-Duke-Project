# ESP32 Modular CLI Interface

This project implements a modular, plug-and-play CLI interface for ESP32 projects using ESP-IDF and FreeRTOS.

## Features

- **Modular Design**: Easy to add/remove command modules
- **ESP-IDF Integration**: Uses ESP-IDF's console utilities and FreeRTOS
- **External Command Support**: Commands can be registered from external folders (TestCases)
- **Built-in Commands**: System information, memory management, task monitoring
- **Utility Commands**: ADC, CAN, I2C, GPIO, DAC operations
- **Test Commands**: Comprehensive test cases for system validation
- **Auto-completion**: TAB completion for commands
- **Command History**: UP/DOWN arrow navigation through command history
- **Colored Output**: ANSI color codes for better readability

## Architecture

```
main/
├── utils/CLI/
│   ├── cli_interface.h/.c    # Core CLI interface
│   └── cli_commands.h/.c     # Utility-specific commands
└── main.c                    # Application entry point

TestCases/
├── test_commands.h/.c        # Test case commands
└── external_commands.c       # External command registration bridge
```

## Quick Start

1. **Initialize CLI in your main application:**
```c
#include "utils/CLI/cli_interface.h"
#include "utils/CLI/cli_commands.h"

void app_main(void) {
    // Initialize CLI
    cli_interface_init(NULL);  // Use default config
    
    // Register utility commands
    cli_register_utility_commands();
    
    // Start CLI
    cli_interface_start();
}
```

2. **Built-in Commands Available:**
   - `help` - Show available commands
   - `version` - System information
   - `free` - Memory usage
   - `heap` - Detailed heap info
   - `tasks` - FreeRTOS task list
   - `restart` - System restart

3. **Utility Commands:**
   - `adc-read -c <channel>` - Read ADC channel
   - `can-send -i <id> -d <data>` - Send CAN message
   - `gpio-set -p <pin> -l <level>` - Set GPIO level
   - `i2c-scan` - Scan I2C bus
   - `dac-set -v <value>` - Set DAC output

4. **Test Commands:**
   - `test-led -p <pin> -d <duration>` - LED blink test
   - `test-sensors` - Sensor validation
   - `test-comm` - Communication interface tests
   - `test-memory` - Memory and heap tests
   - `run-all-tests` - Complete test suite

## Adding External Commands

### Method 1: Through TestCases Directory

1. **Create your command header file:**
```c
// TestCases/my_commands.h
#include "../main/utils/CLI/cli_interface.h"

void register_my_commands(void);
int cmd_my_function(int argc, char **argv);
```

2. **Implement your commands:**
```c
// TestCases/my_commands.c
#include "my_commands.h"

void register_my_commands(void) {
    const cli_command_t my_commands[] = {
        {
            .command = "my-cmd",
            .help = "My custom command",
            .func = cmd_my_function,
            .argtable = NULL
        }
    };
    
    cli_register_commands(my_commands, 1);
}

int cmd_my_function(int argc, char **argv) {
    cli_printf("Hello from my custom command!\n");
    return 0;
}
```

3. **Register in external_commands.c:**
```c
// TestCases/external_commands.c
#include "my_commands.h"

void cli_register_external_commands(void) {
    register_test_commands();
    register_my_commands();  // Add your registration here
}
```

### Method 2: Direct Registration

You can also register commands directly from any part of your application:

```c
#include "utils/CLI/cli_interface.h"

int my_command_handler(int argc, char **argv) {
    cli_printf("Custom command executed!\n");
    return 0;
}

void my_init_function(void) {
    cli_command_t my_cmd = {
        .command = "custom",
        .help = "My custom command",
        .func = my_command_handler,
        .argtable = NULL
    };
    
    cli_register_command(&my_cmd);
}
```

## Command Arguments

Use argtable3 for command argument parsing:

```c
// Define argument structure
static struct {
    struct arg_int *value;
    struct arg_str *name;
    struct arg_end *end;
} my_args;

// Initialize in registration function
my_args.value = arg_int1("v", "value", "<num>", "Integer value");
my_args.name = arg_str0("n", "name", "<str>", "Optional name");
my_args.end = arg_end(3);

// Use in command handler
int cmd_handler(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &my_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, my_args.end, argv[0]);
        return 1;
    }
    
    int value = my_args.value->ival[0];
    const char *name = my_args.name->count > 0 ? my_args.name->sval[0] : "default";
    
    cli_printf("Value: %d, Name: %s\n", value, name);
    return 0;
}
```

## Output Formatting

Use the provided formatting functions for consistent output:

```c
cli_printf("Normal output\n");
cli_printf_success("Operation completed successfully\n");
cli_printf_error("An error occurred\n");
cli_printf_warning("Warning message\n");
```

## Configuration

Customize CLI behavior with configuration:

```c
cli_config_t config = {
    .echo_enabled = true,
    .history_enabled = true,
    .max_cmdline_length = 512,
    .history_save_path = NULL
};

cli_interface_init(&config);
```

## Thread Safety

The CLI interface uses FreeRTOS mutexes for thread safety. All CLI functions are safe to call from multiple tasks.

## Memory Management

- The CLI uses dynamic memory allocation for command registration
- Built-in memory monitoring commands help track heap usage
- Command history is stored in NVS for persistence

## Best Practices

1. **Command Naming**: Use kebab-case (e.g., `test-sensors`)
2. **Help Text**: Always provide clear help descriptions
3. **Error Handling**: Return appropriate error codes (0 = success, non-zero = error)
4. **Argument Validation**: Validate all user inputs
5. **Output Formatting**: Use the provided formatting functions
6. **Resource Cleanup**: Free allocated resources in command handlers

## Integration with Existing Projects

To integrate this CLI system into an existing ESP-IDF project:

1. Copy the `utils/CLI/` directory to your project
2. Add the CLI sources to your CMakeLists.txt
3. Include required ESP-IDF components (esp_console, nvs_flash, driver)
4. Initialize the CLI in your main application
5. Register your project-specific commands

The modular design ensures minimal impact on existing code while providing powerful debugging and testing capabilities.
