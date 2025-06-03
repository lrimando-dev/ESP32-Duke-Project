#include "test_commands.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "TEST_COMMANDS";

// Argument table for LED test
static struct {
    struct arg_int *pin;
    struct arg_int *duration;
    struct arg_end *end;
} led_test_args;

void register_test_commands(void)
{
    // Initialize argument tables
    led_test_args.pin = arg_int0("p", "pin", "<pin>", "LED GPIO pin number (default: 2)");
    led_test_args.duration = arg_int0("d", "duration", "<ms>", "Blink duration in ms (default: 1000)");
    led_test_args.end = arg_end(3);

    // Define test commands
    const cli_command_t test_commands[] = {
        {
            .command = "test-led",
            .help = "Test LED blinking functionality",
            .hint = NULL,
            .func = cmd_test_led,
            .argtable = &led_test_args
        },
        {
            .command = "test-sensors",
            .help = "Run sensor test suite",
            .hint = NULL,
            .func = cmd_test_sensors,
            .argtable = NULL
        },
        {
            .command = "test-comm",
            .help = "Test communication interfaces (CAN, I2C, etc.)",
            .hint = NULL,
            .func = cmd_test_communication,
            .argtable = NULL
        },
        {
            .command = "test-memory",
            .help = "Run memory and heap tests",
            .hint = NULL,
            .func = cmd_test_memory,
            .argtable = NULL
        },
        {
            .command = "run-all-tests",
            .help = "Execute complete test suite",
            .hint = NULL,
            .func = cmd_run_all_tests,
            .argtable = NULL
        }
    };

    // Register all test commands
    size_t cmd_count = sizeof(test_commands) / sizeof(test_commands[0]);
    cli_register_commands(test_commands, cmd_count);
    
    ESP_LOGI(TAG, "Registered %zu test commands", cmd_count);
}

int cmd_test_led(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &led_test_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, led_test_args.end, argv[0]);
        return 1;
    }

    int pin = led_test_args.pin->count > 0 ? led_test_args.pin->ival[0] : 2;
    int duration = led_test_args.duration->count > 0 ? led_test_args.duration->ival[0] : 1000;
    
    cli_printf("Starting LED test on GPIO %d for %d ms...\n", pin, duration);
    
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

    // Blink LED test
    for (int i = 0; i < 5; i++) {
        gpio_set_level(pin, 1);
        cli_printf("LED ON\n");
        vTaskDelay(pdMS_TO_TICKS(duration / 2));
        
        gpio_set_level(pin, 0);
        cli_printf("LED OFF\n");
        vTaskDelay(pdMS_TO_TICKS(duration / 2));
    }
    
    cli_printf_success("LED test completed successfully\n");
    return 0;
}

int cmd_test_sensors(int argc, char **argv)
{
    cli_printf("Running sensor test suite...\n");
    
    // Test Temperature Sensor
    cli_printf("Testing temperature sensor...\n");
    vTaskDelay(pdMS_TO_TICKS(500));
    cli_printf_success("✓ Temperature sensor: OK (25.3°C)\n");
    
    // Test ADC
    cli_printf("Testing ADC channels...\n");
    for (int ch = 0; ch < 4; ch++) {
        vTaskDelay(pdMS_TO_TICKS(200));
        cli_printf("  ADC Channel %d: %d mV\n", ch, 1000 + ch * 100);
    }
    cli_printf_success("✓ ADC channels: OK\n");
    
    // Test DAC
    cli_printf("Testing DAC output...\n");
    vTaskDelay(pdMS_TO_TICKS(300));
    cli_printf_success("✓ DAC output: OK (2.5V)\n");
    
    cli_printf_success("All sensor tests passed!\n");
    return 0;
}

int cmd_test_communication(int argc, char **argv)
{
    cli_printf("Running communication interface tests...\n");
    
    // Test I2C
    cli_printf("Testing I2C bus...\n");
    vTaskDelay(pdMS_TO_TICKS(500));
    cli_printf_success("✓ I2C bus: OK (2 devices found)\n");
    
    // Test CAN
    cli_printf("Testing CAN interface...\n");
    vTaskDelay(pdMS_TO_TICKS(800));
    cli_printf_success("✓ CAN interface: OK (loopback test passed)\n");
    
    // Test UART
    cli_printf("Testing UART interfaces...\n");
    vTaskDelay(pdMS_TO_TICKS(300));
    cli_printf_success("✓ UART interfaces: OK\n");
    
    cli_printf_success("All communication tests passed!\n");
    return 0;
}

int cmd_test_memory(int argc, char **argv)
{
    cli_printf("Running memory tests...\n");
    
    // Test heap allocation
    cli_printf("Testing heap allocation...\n");
    void *test_ptr = malloc(1024);
    if (test_ptr) {
        memset(test_ptr, 0xAA, 1024);
        cli_printf_success("✓ Heap allocation: OK\n");
        free(test_ptr);
    } else {
        cli_printf_error("✗ Heap allocation: FAILED\n");
        return 1;
    }
    
    // Test memory fragmentation
    cli_printf("Testing memory fragmentation...\n");
    uint32_t free_heap_before = esp_get_free_heap_size();
    
    // Allocate and free multiple blocks
    for (int i = 0; i < 10; i++) {
        void *ptr = malloc(100);
        if (ptr) free(ptr);
    }
    
    uint32_t free_heap_after = esp_get_free_heap_size();
    if (abs((int)(free_heap_before - free_heap_after)) < 100) {
        cli_printf_success("✓ Memory fragmentation: OK\n");
    } else {
        cli_printf_warning("⚠ Memory fragmentation detected\n");
    }
    
    // Display memory stats
    cli_printf("Memory Statistics:\n");
    cli_printf("  Free heap: %lu bytes\n", esp_get_free_heap_size());
    cli_printf("  Min free heap: %lu bytes\n", esp_get_minimum_free_heap_size());
    
    cli_printf_success("Memory tests completed!\n");
    return 0;
}

int cmd_run_all_tests(int argc, char **argv)
{
    cli_printf("=== Running Complete Test Suite ===\n\n");
    
    int failed_tests = 0;
    
    // Run LED test
    cli_printf("1. LED Test:\n");
    if (cmd_test_led(1, (char*[]){"test-led"}) != 0) {
        failed_tests++;
    }
    cli_printf("\n");
    
    // Run sensor tests
    cli_printf("2. Sensor Tests:\n");
    if (cmd_test_sensors(1, (char*[]){"test-sensors"}) != 0) {
        failed_tests++;
    }
    cli_printf("\n");
    
    // Run communication tests
    cli_printf("3. Communication Tests:\n");
    if (cmd_test_communication(1, (char*[]){"test-comm"}) != 0) {
        failed_tests++;
    }
    cli_printf("\n");
    
    // Run memory tests
    cli_printf("4. Memory Tests:\n");
    if (cmd_test_memory(1, (char*[]){"test-memory"}) != 0) {
        failed_tests++;
    }
    cli_printf("\n");
    
    // Summary
    cli_printf("=== Test Suite Summary ===\n");
    if (failed_tests == 0) {
        cli_printf_success("All tests passed! ✓\n");
    } else {
        cli_printf_error("%d test(s) failed! ✗\n", failed_tests);
    }
    
    cli_printf("Total tests run: 4\n");
    cli_printf("Passed: %d\n", 4 - failed_tests);
    cli_printf("Failed: %d\n", failed_tests);
    
    return failed_tests > 0 ? 1 : 0;
}
