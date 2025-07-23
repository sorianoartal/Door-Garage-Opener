#include "utils/HelperFunc.h"



/**
 * @brief Function to print a specified number of dots with a delay between each dot.
 * * This function is useful for indicating progress in a process, such as during initialization or data processing.
 * 
 * @param numOfDots - The number of dots to print.
 * @param delay_ms - The delay in milliseconds between each dot.
 * 
 * @code 
 * // Example usage:
 * printDots(5, 500); // Prints 5 dots with a 500 ms delay between each dot.
 * @endcode
 * * @note This function is usefull to add a delay between operations, such as during initialization or data processing. So is easy to see the progress in the serial monitor.
 */
void printDots(uint8_t numOfDots, unsigned long delay_ms)
{
    // Validate delay_ms
    if(delay_ms == 0)
    {
        LOG_NEW_LINE("Error: delay_ms cannot be zero");
        return;
    }

    // Lambda function to print a dot and reset the timer
    // This function will be called repeatedly to print dots
    auto printDot = [&]()
    {
        LOG(".");
        delay(delay_ms);  // Wait for the specified delay
    };

    // Prints numOfDots dots with the specified timeout
    avr_algorithms::repeat(numOfDots,printDot);

    LOG("\n\n"); // Print a new line after all dots
}


/**
 * @brief Function to wait for a specified delay in milliseconds.
 * * This function is useful for adding a delay between operations, such as during initialization or data processing.
 * 
 * @param delay_ms - The delay in milliseconds to wait.
 * * @code
 * // Example usage:
 * wait(1000); // Waits for 1000 milliseconds (1 second).
 * @endcode
 * * @note This function is useful to add a delay between operations, such as during initialization or data processing.
 */
void wait(unsigned long delay_ms)
{
    unsigned long startTime = millis();  // Get the current time

    while (millis() - startTime < delay_ms) {
        // Do nothing, just wait
    }     
}