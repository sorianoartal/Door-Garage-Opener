#pragma once

#include<Arduino.h>

/**
 * @brief Interface that all Debounce method must implement 
 * 
 */
class IDebounce
{
public:

    virtual ~IDebounce() = default ;                      // Default destructor to ensure proper cleanup

    virtual void startDebounce() = 0;                     // Arm a Debounce session when button press is detected 
    virtual void update() = 0 ;                              // Method that get called from the main to update the debounce logic : Proccess the buffer to determine the stable state 
    virtual bool getStableState() const = 0 ;          // Returns true if the switch is stably pressed, false otherwise
    virtual void addSample(bool state) = 0 ;         // Add new reading to the circular buffer
    virtual void reset() = 0 ;                                 // Reset Functionality that will vary depending of the type of debounce method used 
};