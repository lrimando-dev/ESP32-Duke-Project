#include "performance_commands.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

static const char *TAG = "PERF_COMMANDS";

// Argument tables
static struct {
    struct arg_int *iterations;
    struct arg_end *end;
} cpu_benchmark_args;

static struct {
    struct arg_int *size;
    struct arg_int *count;
    struct arg_end *end;
} memory_benchmark_args;

static struct {
    struct arg_int *duration;
    struct arg_end *end;
} stress_test_args;

void register_performance_commands(void)
{
    // Initialize argument tables
    cpu_benchmark_args.iterations = arg_int0("i", "iterations", "<num>", "Number of iterations (default: 10000)");
    cpu_benchmark_args.end = arg_end(2);

    memory_benchmark_args.size = arg_int0("s", "size", "<bytes>", "Block size in bytes (default: 1024)");
    memory_benchmark_args.count = arg_int0("c", "count", "<num>", "Number of blocks (default: 100)");
    memory_benchmark_args.end = arg_end(3);

    stress_test_args.duration = arg_int0("d", "duration", "<seconds>", "Test duration in seconds (default: 10)");
    stress_test_args.end = arg_end(2);

    // Define performance commands
    const cli_command_t perf_commands[] = {
        {
            .command = "bench-cpu",
            .help = "Run CPU benchmark test",
            .hint = NULL,
            .func = cmd_benchmark_cpu,
            .argtable = &cpu_benchmark_args
        },
        {
            .command = "bench-memory",
            .help = "Run memory benchmark test",
            .hint = NULL,
            .func = cmd_benchmark_memory,
            .argtable = &memory_benchmark_args
        },
        {
            .command = "stress-test",
            .help = "Run system stress test",
            .hint = NULL,
            .func = cmd_stress_test,
            .argtable = &stress_test_args
        },
        {
            .command = "profile-tasks",
            .help = "Profile FreeRTOS task performance",
            .hint = NULL,
            .func = cmd_profile_tasks,
            .argtable = NULL
        }
    };

    // Register all performance commands
    size_t cmd_count = sizeof(perf_commands) / sizeof(perf_commands[0]);
    cli_register_commands(perf_commands, cmd_count);
    
    ESP_LOGI(TAG, "Registered %zu performance commands", cmd_count);
}

int cmd_benchmark_cpu(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &cpu_benchmark_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, cpu_benchmark_args.end, argv[0]);
        return 1;
    }

    int iterations = cpu_benchmark_args.iterations->count > 0 ? 
                    cpu_benchmark_args.iterations->ival[0] : 10000;
    
    cli_printf("Running CPU benchmark with %d iterations...\n", iterations);
    
    // Record start time
    int64_t start_time = esp_timer_get_time();
    
    // CPU intensive calculation (prime number generation)
    int prime_count = 0;
    for (int n = 2; n <= iterations; n++) {
        bool is_prime = true;
        for (int i = 2; i <= sqrt(n); i++) {
            if (n % i == 0) {
                is_prime = false;
                break;
            }
        }
        if (is_prime) {
            prime_count++;
        }
    }
    
    // Record end time
    int64_t end_time = esp_timer_get_time();
    int64_t duration_us = end_time - start_time;
    
    cli_printf("CPU Benchmark Results:\n");
    cli_printf("  Iterations: %d\n", iterations);
    cli_printf("  Prime numbers found: %d\n", prime_count);
    cli_printf("  Duration: %lld µs (%.2f ms)\n", duration_us, duration_us / 1000.0);
    cli_printf("  Rate: %.2f iterations/ms\n", (float)iterations / (duration_us / 1000.0));
    
    cli_printf_success("CPU benchmark completed\n");
    return 0;
}

int cmd_benchmark_memory(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &memory_benchmark_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, memory_benchmark_args.end, argv[0]);
        return 1;
    }

    int block_size = memory_benchmark_args.size->count > 0 ? 
                    memory_benchmark_args.size->ival[0] : 1024;
    int block_count = memory_benchmark_args.count->count > 0 ? 
                     memory_benchmark_args.count->ival[0] : 100;
    
    cli_printf("Running memory benchmark: %d blocks of %d bytes...\n", block_count, block_size);
    
    // Allocation benchmark
    int64_t alloc_start = esp_timer_get_time();
    void **ptrs = malloc(block_count * sizeof(void*));
    if (!ptrs) {
        cli_printf_error("Failed to allocate pointer array\n");
        return 1;
    }
    
    // Allocate blocks
    int successful_allocs = 0;
    for (int i = 0; i < block_count; i++) {
        ptrs[i] = malloc(block_size);
        if (ptrs[i]) {
            successful_allocs++;
        }
    }
    int64_t alloc_end = esp_timer_get_time();
    
    // Write benchmark
    int64_t write_start = esp_timer_get_time();
    for (int i = 0; i < successful_allocs; i++) {
        if (ptrs[i]) {
            memset(ptrs[i], 0xAA, block_size);
        }
    }
    int64_t write_end = esp_timer_get_time();
    
    // Read benchmark
    int64_t read_start = esp_timer_get_time();
    volatile uint8_t dummy = 0;
    for (int i = 0; i < successful_allocs; i++) {
        if (ptrs[i]) {
            uint8_t *data = (uint8_t*)ptrs[i];
            for (int j = 0; j < block_size; j += 64) { // Sample every 64 bytes
                dummy += data[j];
            }
        }
    }
    int64_t read_end = esp_timer_get_time();
    
    // Free benchmark
    int64_t free_start = esp_timer_get_time();
    for (int i = 0; i < block_count; i++) {
        if (ptrs[i]) {
            free(ptrs[i]);
        }
    }
    free(ptrs);
    int64_t free_end = esp_timer_get_time();
    
    // Calculate rates
    int64_t total_bytes = (int64_t)successful_allocs * block_size;
    
    cli_printf("Memory Benchmark Results:\n");
    cli_printf("  Block size: %d bytes\n", block_size);
    cli_printf("  Requested blocks: %d\n", block_count);
    cli_printf("  Successful allocations: %d\n", successful_allocs);
    cli_printf("  Total memory: %lld bytes (%.2f KB)\n", total_bytes, total_bytes / 1024.0);
    cli_printf("\n");
    cli_printf("  Allocation time: %lld µs (%.2f MB/s)\n", 
               alloc_end - alloc_start,
               total_bytes / ((alloc_end - alloc_start) / 1000000.0) / (1024 * 1024));
    cli_printf("  Write time: %lld µs (%.2f MB/s)\n", 
               write_end - write_start,
               total_bytes / ((write_end - write_start) / 1000000.0) / (1024 * 1024));
    cli_printf("  Read time: %lld µs (%.2f MB/s)\n", 
               read_end - read_start,
               total_bytes / ((read_end - read_start) / 1000000.0) / (1024 * 1024));
    cli_printf("  Free time: %lld µs\n", free_end - free_start);
    
    (void)dummy; // Suppress unused variable warning
    
    cli_printf_success("Memory benchmark completed\n");
    return 0;
}

int cmd_stress_test(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &stress_test_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, stress_test_args.end, argv[0]);
        return 1;
    }

    int duration_seconds = stress_test_args.duration->count > 0 ? 
                          stress_test_args.duration->ival[0] : 10;
    
    cli_printf("Running stress test for %d seconds...\n", duration_seconds);
    cli_printf("Press Ctrl+C to stop early\n");
    
    uint32_t start_heap = esp_get_free_heap_size();
    uint32_t min_heap = start_heap;
    int64_t start_time = esp_timer_get_time();
    int64_t end_time = start_time + (duration_seconds * 1000000LL);
    
    int cycles = 0;
    int allocation_failures = 0;
    
    while (esp_timer_get_time() < end_time) {
        // Allocate and free memory blocks
        for (int i = 0; i < 10; i++) {
            size_t size = 100 + (esp_random() % 900); // 100-1000 bytes
            void *ptr = malloc(size);
            if (ptr) {
                memset(ptr, 0x55, size);
                vTaskDelay(1); // Brief delay
                free(ptr);
            } else {
                allocation_failures++;
            }
        }
        
        // CPU intensive task
        volatile float result = 0;
        for (int i = 0; i < 1000; i++) {
            result += sin(i * 0.1) * cos(i * 0.2);
        }
        
        // Monitor heap
        uint32_t current_heap = esp_get_free_heap_size();
        if (current_heap < min_heap) {
            min_heap = current_heap;
        }
        
        cycles++;
        
        // Progress update every second
        int64_t elapsed = esp_timer_get_time() - start_time;
        if ((elapsed / 1000000) > (cycles / 100)) {
            cli_printf(".");
            fflush(stdout);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    cli_printf("\n\nStress Test Results:\n");
    cli_printf("  Duration: %d seconds\n", duration_seconds);
    cli_printf("  Cycles completed: %d\n", cycles);
    cli_printf("  Allocation failures: %d\n", allocation_failures);
    cli_printf("  Starting heap: %lu bytes\n", start_heap);
    cli_printf("  Minimum heap: %lu bytes\n", min_heap);
    cli_printf("  Ending heap: %lu bytes\n", esp_get_free_heap_size());
    cli_printf("  Heap usage: %lu bytes\n", start_heap - min_heap);
    
    if (allocation_failures == 0 && esp_get_free_heap_size() >= (start_heap - 1000)) {
        cli_printf_success("Stress test PASSED\n");
        return 0;
    } else {
        cli_printf_warning("Stress test completed with issues\n");
        return 1;
    }
}

int cmd_profile_tasks(int argc, char **argv)
{
    cli_printf("Profiling FreeRTOS tasks...\n");
    
    // Get task information
    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    TaskStatus_t *task_array = malloc(task_count * sizeof(TaskStatus_t));
    
    if (!task_array) {
        cli_printf_error("Failed to allocate memory for task array\n");
        return 1;
    }
    
    uint32_t total_runtime;
    UBaseType_t actual_count = uxTaskGetSystemState(task_array, task_count, &total_runtime);
    
    cli_printf("\nTask Performance Profile:\n");
    cli_printf("%-16s %8s %8s %8s %8s %6s\n", 
               "Name", "State", "Priority", "Stack", "Runtime", "CPU%");
    cli_printf("================================================================\n");
    
    for (UBaseType_t i = 0; i < actual_count; i++) {
        TaskStatus_t *task = &task_array[i];
        
        const char *state_str;
        switch (task->eCurrentState) {
            case eRunning:   state_str = "Running"; break;
            case eReady:     state_str = "Ready"; break;
            case eBlocked:   state_str = "Blocked"; break;
            case eSuspended: state_str = "Suspend"; break;
            case eDeleted:   state_str = "Deleted"; break;
            default:         state_str = "Unknown"; break;
        }
        
        float cpu_percent = 0.0;
        if (total_runtime > 0) {
            cpu_percent = (float)(task->ulRunTimeCounter * 100) / total_runtime;
        }
        
        cli_printf("%-16s %8s %8lu %8lu %8lu %5.1f%%\n",
                   task->pcTaskName,
                   state_str,
                   task->uxCurrentPriority,
                   task->usStackHighWaterMark,
                   task->ulRunTimeCounter,
                   cpu_percent);
    }
    
    cli_printf("\nSystem Summary:\n");
    cli_printf("  Total tasks: %lu\n", actual_count);
    cli_printf("  Total runtime: %lu ticks\n", total_runtime);
    cli_printf("  Free heap: %lu bytes\n", esp_get_free_heap_size());
    
    free(task_array);
    cli_printf_success("Task profiling completed\n");
    return 0;
}
