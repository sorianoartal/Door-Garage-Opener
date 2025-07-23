#pragma once

#include <Arduino.h>
#include "interfaces/IDebounce.h"
#include "Delay/Delay.h"
#include "avr_algorithms.hpp"
#include "Debugging/Logging.h"


// How many samples we keep in the circular buffer (16 is typical)
static constexpr size_t BUFFER_SIZE = 16;

// Maximum number of zero‐arg callbacks we support
static constexpr size_t MAX_CALLBACKS = 4;

// Callback type: zero arguments
using Callback = void(*)();

class CircularDebounceBuffer {
public:
    /**
     * @param id                An arbitrary ID (not used by zero‐arg callbacks)
     * @param pin               The Arduino digital pin to debounce
     * @param isActiveLow       If true, a LOW read means “pressed”
     * @param delayBetweenUs    How many microseconds between each raw sample
     */
    CircularDebounceBuffer(
        uint8_t id,
        uint8_t pin,
        bool    isActiveLow = true,
        uint32_t delayBetweenUs = 1000
    );

    // Register a zero‐argument callback (called once per confirmed press)
    void addCallback(Callback cb);

    // Set the percentage of BUFFER_SIZE that must be “true” to confirm a press
    void setThreshold(uint8_t percentage);

    // Change how many microseconds between samples
    void setSampleIntervalUs(uint32_t newDelayUs);

    // Called repeatedly (in loop()). Samples only once per interval.
    void update();

    // Arms debounce on the FALLING ISR to start a new debounce session
    void startDebounce();

    // Query the last debounced stable state (true=pressed, false=released)
    bool getStableState() const;

    // Re‐initialize everything (buffer, flags, timer). Useful if you want to reset.
    void reset();

private:

    uint8_t   _pin_ID;                                     // arbitrary ID (unused in zero‐arg callback)
    uint8_t   _pin;                                         // the Arduino pin number
    bool      _isActiveLow;                            // if true, LOW means “pressed”
    bool      _debouncing;                           // true while we’re in a press/release cycle
    bool      _stableState;                            // last confirmed stable state (true=pressed)
    bool      _pressedDetected;                   // set true once we have fired the press callback
    bool      _buffer[BUFFER_SIZE];               // circular buffer of last BUFFER_SIZE samples
    size_t    _head;                                      // index of next slot to overwrite
    uint8_t   _thresholdPercentage;               // e.g. 60 means 60% of BUFFER_SIZE
    Callback  _callbacks[MAX_CALLBACKS];
    size_t    _callbackCounter;

    Delay     _delayBetweenSamples;   // Delay object (micros‐based)

    // Zero out the buffer and reset head to 0
    void clearBuffer();
};