#pragma once

#include <Arduino.h>

/**
 * @brief Defines the interface for the bit_level behavior that each Encoder class must implement to construct a waveform
 * 
 */
class IBitEncoder 
{
    public:

    ~IBitEncoder() = default;

    virtual void sendOne() = 0;             // Send a logic '1'
    virtual void sendZero() = 0;            // Send a logic '0'
    virtual void sendOpen() = 0;           // For a tri-state encoding special digits  
    virtual void sendSilence() = 0;         // Silence send (LOW) between two repeated consecutive words
    virtual void sendPreamble() = 0 ;    // For synchronizing the receiver's internal state logic
    virtual void setIdle() = 0;                // Set output to the specific RF protocol’s resting state.
};