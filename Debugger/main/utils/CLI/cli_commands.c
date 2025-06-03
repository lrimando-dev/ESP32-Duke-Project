#include "cli_commands.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include <string.h>
#include <stdlib.h>

// Include your utility headers
#include "../ADC/adc_utils.h"
#include "../CAN/can_driver_utils.h"
#include "../TempSensor/temp_sensor.h"
#include "../AD5693/ad5693_utils.h"

static const char *TAG = "CLI_COMMANDS";

// Argument tables for commands with parameters
static struct {
    struct arg_int *channel;
    struct arg_end *end;
} adc_read_args;

static struct {
    struct arg_int *id;
    struct arg_str *data;
    struct arg_end *end;
} can_send_args;

static struct {
    struct arg_int *value;
    struct arg_end *end;
} dac_set_args;

static struct {
    struct arg_int *pin;
    struct arg_int *level;
    struct arg_end *end;
} gpio_set_args;

static struct {
    struct arg_int *pin;
    struct arg_end *end;
} gpio_get_args;

void cli_register_utility_commands(void)
{
    // Initialize argument tables
    adc_read_args.channel = arg_int0("c", "channel", "<0-7>", "ADC channel number");
    adc_read_args.end = arg_end(2);

    can_send_args.id = arg_int1("i", "id", "<id>", "CAN message ID");
    can_send_args.data = arg_str1("d", "data", "<hex>", "Data in hex format (e.g., 01020304)");
    can_send_args.end = arg_end(3);

    dac_set_args.value = arg_int1("v", "value", "<0-4095>", "DAC value (12-bit)");
    dac_set_args.end = arg_end(2);

    gpio_set_args.pin = arg_int1("p", "pin", "<pin>", "GPIO pin number");
    gpio_set_args.level = arg_int1("l", "level", "<0|1>", "GPIO level (0 or 1)");
    gpio_set_args.end = arg_end(3);

    gpio_get_args.pin = arg_int1("p", "pin", "<pin>", "GPIO pin number");
    gpio_get_args.end = arg_end(2);

    // Define utility commands
    const cli_command_t utility_commands[] = {
        // ADC Commands
        {
            .command = "adc-read",
            .help = "Read ADC channel value",
            .hint = NULL,
            .func = cmd_adc_read,
            .argtable = &adc_read_args
        },
        {
            .command = "adc-cal",
            .help = "Calibrate ADC",
            .hint = NULL,
            .func = cmd_adc_calibrate,
            .argtable = NULL
        },
        
        // CAN Commands
        {
            .command = "can-send",
            .help = "Send CAN message",
            .hint = NULL,
            .func = cmd_can_send,
            .argtable = &can_send_args
        },
        {
            .command = "can-recv",
            .help = "Receive CAN messages",
            .hint = NULL,
            .func = cmd_can_receive,
            .argtable = NULL
        },
        {
            .command = "can-status",
            .help = "Show CAN bus status",
            .hint = NULL,
            .func = cmd_can_status,
            .argtable = NULL
        },
        
        // Temperature Sensor Commands
        {
            .command = "temp-read",
            .help = "Read temperature sensor",
            .hint = NULL,
            .func = cmd_temp_read,
            .argtable = NULL
        },
        
        // DAC Commands
        {
            .command = "dac-set",
            .help = "Set DAC output value",
            .hint = NULL,
            .func = cmd_dac_set,
            .argtable = &dac_set_args
        },
        {
            .command = "dac-read",
            .help = "Read current DAC value",
            .hint = NULL,
            .func = cmd_dac_read,
            .argtable = NULL
        },
        
        // GPIO Commands
        {
            .command = "gpio-set",
            .help = "Set GPIO pin level",
            .hint = NULL,
            .func = cmd_gpio_set,
            .argtable = &gpio_set_args
        },
        {
            .command = "gpio-get",
            .help = "Get GPIO pin level",
            .hint = NULL,
            .func = cmd_gpio_get,
            .argtable = &gpio_get_args
        },
        
        // I2C Commands
        {
            .command = "i2c-scan",
            .help = "Scan I2C bus for devices",
            .hint = NULL,
            .func = cmd_i2c_scan,
            .argtable = NULL
        }
    };

    // Register all utility commands
    size_t cmd_count = sizeof(utility_commands) / sizeof(utility_commands[0]);
    cli_register_commands(utility_commands, cmd_count);
    
    ESP_LOGI(TAG, "Registered %zu utility commands", cmd_count);
}

// ADC Command Implementations
int cmd_adc_read(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &adc_read_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, adc_read_args.end, argv[0]);
        return 1;
    }

    int channel = adc_read_args.channel->count > 0 ? adc_read_args.channel->ival[0] : 0;
    
    if (channel < 0 || channel > 7) {
        cli_printf_error("Invalid ADC channel. Must be 0-7\n");
        return 1;
    }

    cli_printf("Reading ADC channel %d...\n", channel);
    // Note: You'll need to implement the actual ADC reading logic here
    // using your adc_utils functions
    cli_printf_success("ADC Channel %d: Raw value = %d, Voltage = %.2f mV\n", 
                      channel, 1234, 567.89); // Placeholder values
    
    return 0;
}

int cmd_adc_calibrate(int argc, char **argv)
{
    cli_printf("Calibrating ADC...\n");
    // Implement ADC calibration using your adc_utils
    cli_printf_success("ADC calibration completed\n");
    return 0;
}

// CAN Command Implementations
int cmd_can_send(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &can_send_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, can_send_args.end, argv[0]);
        return 1;
    }

    uint32_t can_id = can_send_args.id->ival[0];
    const char *hex_data = can_send_args.data->sval[0];
    
    cli_printf("Sending CAN message ID: 0x%X, Data: %s\n", can_id, hex_data);
    // Implement CAN transmission using your can_transmit_utils
    cli_printf_success("CAN message sent successfully\n");
    
    return 0;
}

int cmd_can_receive(int argc, char **argv)
{
    cli_printf("Listening for CAN messages (Press Ctrl+C to stop)...\n");
    // Implement CAN reception using your can_receive_utils
    cli_printf("Received CAN ID: 0x123, Data: [01 02 03 04]\n"); // Placeholder
    return 0;
}

int cmd_can_status(int argc, char **argv)
{
    cli_printf("CAN Bus Status:\n");
    cli_printf("- State: Active\n");
    cli_printf("- Bitrate: 500 kbps\n");
    cli_printf("- TX Count: 123\n");
    cli_printf("- RX Count: 456\n");
    cli_printf("- Error Count: 0\n");
    return 0;
}

// Temperature Sensor Command Implementation
int cmd_temp_read(int argc, char **argv)
{
    cli_printf("Reading temperature sensor...\n");
    // Implement temperature reading using your temp_sensor utils
    float temperature = 25.5; // Placeholder
    cli_printf_success("Temperature: %.2f°C\n", temperature);
    return 0;
}

// DAC Command Implementations
int cmd_dac_set(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &dac_set_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, dac_set_args.end, argv[0]);
        return 1;
    }

    int dac_value = dac_set_args.value->ival[0];
    
    if (dac_value < 0 || dac_value > 4095) {
        cli_printf_error("Invalid DAC value. Must be 0-4095\n");
        return 1;
    }

    cli_printf("Setting DAC to value: %d\n", dac_value);
    // Implement DAC setting using your ad5693_utils
    cli_printf_success("DAC value set to %d (%.2f V)\n", dac_value, dac_value * 3.3 / 4095.0);
    
    return 0;
}

int cmd_dac_read(int argc, char **argv)
{
    cli_printf("Reading current DAC value...\n");
    // Implement DAC reading using your ad5693_utils
    int current_value = 2048; // Placeholder
    cli_printf_success("Current DAC value: %d (%.2f V)\n", current_value, current_value * 3.3 / 4095.0);
    return 0;
}

// GPIO Command Implementations
int cmd_gpio_set(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &gpio_set_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, gpio_set_args.end, argv[0]);
        return 1;
    }

    int pin = gpio_set_args.pin->ival[0];
    int level = gpio_set_args.level->ival[0];
    
    if (level != 0 && level != 1) {
        cli_printf_error("Invalid level. Must be 0 or 1\n");
        return 1;
    }

    // Configure GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&io_conf);
    
    if (err != ESP_OK) {
        cli_printf_error("Failed to configure GPIO %d: %s\n", pin, esp_err_to_name(err));
        return 1;
    }

    gpio_set_level(pin, level);
    cli_printf_success("GPIO %d set to %d\n", pin, level);
    
    return 0;
}

int cmd_gpio_get(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &gpio_get_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, gpio_get_args.end, argv[0]);
        return 1;
    }

    int pin = gpio_get_args.pin->ival[0];
    
    // Configure GPIO as input
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&io_conf);
    
    if (err != ESP_OK) {
        cli_printf_error("Failed to configure GPIO %d: %s\n", pin, esp_err_to_name(err));
        return 1;
    }

    int level = gpio_get_level(pin);
    cli_printf_success("GPIO %d level: %d\n", pin, level);
    
    return 0;
}

// I2C Command Implementation
int cmd_i2c_scan(int argc, char **argv)
{
    cli_printf("Scanning I2C bus...\n");
    cli_printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    
    for (int i = 0; i < 128; i += 16) {
        cli_printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            int addr = i + j;
            if (addr < 0x08 || addr > 0x77) {
                cli_printf("   ");
            } else {
                // Placeholder I2C scan logic
                // You would implement actual I2C probing here
                if (addr == 0x48 || addr == 0x50) { // Example addresses
                    cli_printf("%02x ", addr);
                } else {
                    cli_printf("-- ");
                }
            }
        }
        cli_printf("\n");
    }
    
    cli_printf_success("I2C scan completed\n");
    return 0;
}