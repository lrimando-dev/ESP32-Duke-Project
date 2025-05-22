// This header file declares the functions and constants used in dac.c.
// It includes function prototypes for initializing the DAC and setting the output value.

#ifndef AD5693_UTILS_H
#define AD5693_UTILS_H

#include <stdint.h>

// Function to initialize the DAC
void dac_init(void);

// Function to set the output value of the DAC
void dac_set_value(uint16_t value);

#endif // AD5693_UTILS_H