# ESP32 CLI Usage Examples

This document provides practical examples of using the modular CLI interface.

## Starting the CLI

When your ESP32 boots, you'll see:

```
I (xxx) MAIN: Starting ESP32 Duke Project Debugger
I (xxx) CLI_INTERFACE: CLI interface initialized successfully
I (xxx) CLI_COMMANDS: Registered 11 utility commands
I (xxx) TEST_COMMANDS: Registered 5 test commands
I (xxx) PERF_COMMANDS: Registered 4 performance commands
I (xxx) CLI_INTERFACE: CLI interface started
I (xxx) MAIN: System initialization complete. CLI interface is running.
I (xxx) MAIN: Type 'help' to see available commands.

===========================================
       ESP32 CLI Interface Ready
===========================================
Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.

ESP32-CLI> 
```

## Basic Commands

### Get Help
```bash
ESP32-CLI> help
Available commands:
  help
  version
  restart
  free
  heap
  tasks
  adc-read
  adc-cal
  can-send
  can-recv
  can-status
  temp-read
  dac-set
  dac-read
  gpio-set
  gpio-get
  i2c-scan
  test-led
  test-sensors
  test-comm
  test-memory
  run-all-tests
  bench-cpu
  bench-memory
  stress-test
  profile-tasks

ESP32-CLI> help version
Command: version
Description: Show system version information
```

### System Information
```bash
ESP32-CLI> version
System Information:
ESP-IDF Version: v5.1.2
Chip: esp32
Silicon revision: 3
Cores: 2
Features: /802.11bgn/BLE/Embedded-Flash:4 MB

ESP32-CLI> free
Memory Information:
Free heap: 245760 bytes
Minimum free heap: 245760 bytes
Heap usage: 0.0%

ESP32-CLI> tasks
FreeRTOS Task Information:
Task Name		State	Prio	Stack	Num
=================================================
IDLE            R       0       896     3
IDLE            R       0       896     4
cli_task        R       5       3584    5
main            S       1       2816    1
ipc0            S       24      1024    2
ipc1            S       24      1024    6
```

## Hardware Testing

### GPIO Operations
```bash
ESP32-CLI> gpio-set -p 2 -l 1
[SUCCESS] GPIO 2 set to 1

ESP32-CLI> gpio-get -p 2
[SUCCESS] GPIO 2 level: 1

ESP32-CLI> gpio-set -p 2 -l 0
[SUCCESS] GPIO 2 set to 0
```

### ADC Reading
```bash
ESP32-CLI> adc-read -c 0
Reading ADC channel 0...
[SUCCESS] ADC Channel 0: Raw value = 1234, Voltage = 567.89 mV

ESP32-CLI> adc-cal
Calibrating ADC...
[SUCCESS] ADC calibration completed
```

### I2C Bus Scan
```bash
ESP32-CLI> i2c-scan
Scanning I2C bus...
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- -- 
50: 50 -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- -- 
[SUCCESS] I2C scan completed
```

### DAC Operations
```bash
ESP32-CLI> dac-set -v 2048
Setting DAC to value: 2048
[SUCCESS] DAC value set to 2048 (1.65 V)

ESP32-CLI> dac-read
Reading current DAC value...
[SUCCESS] Current DAC value: 2048 (1.65 V)
```

## CAN Bus Testing

### Send CAN Message
```bash
ESP32-CLI> can-send -i 0x123 -d 01020304
Sending CAN message ID: 0x123, Data: 01020304
[SUCCESS] CAN message sent successfully

ESP32-CLI> can-status
CAN Bus Status:
- State: Active
- Bitrate: 500 kbps
- TX Count: 123
- RX Count: 456
- Error Count: 0
```

## Test Suites

### LED Test
```bash
ESP32-CLI> test-led -p 2 -d 500
Starting LED test on GPIO 2 for 500 ms...
LED ON
LED OFF
LED ON
LED OFF
LED ON
LED OFF
LED ON
LED OFF
LED ON
LED OFF
[SUCCESS] LED test completed successfully
```

### Sensor Tests
```bash
ESP32-CLI> test-sensors
Running sensor test suite...
Testing temperature sensor...
[SUCCESS] ✓ Temperature sensor: OK (25.3°C)
Testing ADC channels...
  ADC Channel 0: 1000 mV
  ADC Channel 1: 1100 mV
  ADC Channel 2: 1200 mV
  ADC Channel 3: 1300 mV
[SUCCESS] ✓ ADC channels: OK
Testing DAC output...
[SUCCESS] ✓ DAC output: OK (2.5V)
[SUCCESS] All sensor tests passed!
```

### Communication Tests
```bash
ESP32-CLI> test-comm
Running communication interface tests...
Testing I2C bus...
[SUCCESS] ✓ I2C bus: OK (2 devices found)
Testing CAN interface...
[SUCCESS] ✓ CAN interface: OK (loopback test passed)
Testing UART interfaces...
[SUCCESS] ✓ UART interfaces: OK
[SUCCESS] All communication tests passed!
```

### Complete Test Suite
```bash
ESP32-CLI> run-all-tests
=== Running Complete Test Suite ===

1. LED Test:
Starting LED test on GPIO 2 for 1000 ms...
[SUCCESS] LED test completed successfully

2. Sensor Tests:
Running sensor test suite...
[SUCCESS] All sensor tests passed!

3. Communication Tests:
Running communication interface tests...
[SUCCESS] All communication tests passed!

4. Memory Tests:
Running memory tests...
[SUCCESS] Memory tests completed!

=== Test Suite Summary ===
[SUCCESS] All tests passed! ✓
Total tests run: 4
Passed: 4
Failed: 0
```

## Performance Testing

### CPU Benchmark
```bash
ESP32-CLI> bench-cpu -i 5000
Running CPU benchmark with 5000 iterations...
CPU Benchmark Results:
  Iterations: 5000
  Prime numbers found: 669
  Duration: 245892 µs (245.89 ms)
  Rate: 20.33 iterations/ms
[SUCCESS] CPU benchmark completed
```

### Memory Benchmark
```bash
ESP32-CLI> bench-memory -s 512 -c 50
Running memory benchmark: 50 blocks of 512 bytes...
Memory Benchmark Results:
  Block size: 512 bytes
  Requested blocks: 50
  Successful allocations: 50
  Total memory: 25600 bytes (25.00 KB)

  Allocation time: 1234 µs (19.76 MB/s)
  Write time: 2345 µs (10.40 MB/s)
  Read time: 1876 µs (13.01 MB/s)
  Free time: 876 µs
[SUCCESS] Memory benchmark completed
```

### Stress Test
```bash
ESP32-CLI> stress-test -d 5
Running stress test for 5 seconds...
Press Ctrl+C to stop early
.....

Stress Test Results:
  Duration: 5 seconds
  Cycles completed: 487
  Allocation failures: 0
  Starting heap: 245760 bytes
  Minimum heap: 244832 bytes
  Ending heap: 245760 bytes
  Heap usage: 928 bytes
[SUCCESS] Stress test PASSED
```

### Task Profiling
```bash
ESP32-CLI> profile-tasks
Profiling FreeRTOS tasks...

Task Performance Profile:
Name             State Priority    Stack Runtime  CPU%
================================================================
IDLE             Ready        0      896    12345  15.2%
IDLE             Ready        0      896     8765  10.8%
cli_task         Running      5     3584    45678  56.3%
main             Blocked      1     2816     1234   1.5%
ipc0             Blocked     24     1024      567   0.7%
ipc1             Blocked     24     1024      432   0.5%

System Summary:
  Total tasks: 6
  Total runtime: 81234 ticks
  Free heap: 245760 bytes
[SUCCESS] Task profiling completed
```

## Advanced Usage

### Command History
Use UP/DOWN arrow keys to navigate through command history:
```bash
ESP32-CLI> version
ESP32-CLI> ↑  # Shows: version
ESP32-CLI> ↑  # Shows previous command
```

### Tab Completion
Type partial commands and press TAB:
```bash
ESP32-CLI> test-[TAB]
test-led    test-sensors    test-comm    test-memory

ESP32-CLI> bench-[TAB]
bench-cpu    bench-memory
```

### Command Arguments Help
Use incorrect arguments to see usage:
```bash
ESP32-CLI> gpio-set
usage: gpio-set [-p <pin>] [-l <0|1>]
gpio-set: error: option requires an argument -- 'p'

ESP32-CLI> dac-set -v 5000
[ERROR] Invalid DAC value. Must be 0-4095
```

## Integration Examples

### Custom Command Integration
To add your own commands, create files in TestCases/:

```c
// TestCases/my_commands.h
void register_my_commands(void);
int cmd_my_test(int argc, char **argv);

// TestCases/my_commands.c
void register_my_commands(void) {
    const cli_command_t my_commands[] = {
        {
            .command = "my-test",
            .help = "My custom test command",
            .func = cmd_my_test,
            .argtable = NULL
        }
    };
    cli_register_commands(my_commands, 1);
}

// TestCases/external_commands.c
#include "my_commands.h"
void cli_register_external_commands(void) {
    register_test_commands();
    register_performance_commands();
    register_my_commands();  // Add your commands here
}
```

This modular CLI system provides a powerful, extensible interface for ESP32 development, testing, and debugging.
