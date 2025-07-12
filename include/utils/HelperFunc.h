#pragma once

#include <Arduino.h>
#include "avr_algorithms.hpp"
#include "Debugging/Logging.h"

/// Function to print a specified number of dots with a delay between each dot
/// This function is useful for indicating progress in a process, such as during initialization or data processing  
void printDots(uint8_t numOfDots, unsigned long delay_ms);

/// Function to wait for a specified delay in milliseconds
/// This function is useful for adding a delay between operations, such as during initialization or data processing
void wait(unsigned long delay_ms);