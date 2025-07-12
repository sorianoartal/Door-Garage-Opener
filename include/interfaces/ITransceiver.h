#pragma once

#include <stdint.h>

/**
 * @brief Classes that implement this interface are responsible for controlling the physical RF transceiver hardWare
 * 
 */
class ITransceiver
{
    public:
    // Destructor
    ~ITransceiver() = default;

    virtual bool begin() = 0 ;                                                      // Initialize hardware: SPI,  Apply full config passed at construction.
    virtual void setFrequency(uint32_t frequencyHz) = 0;              // Write the frequency registers to tune which is gonna be the carrier freq. used
    virtual void setPowerLevel(uint8_t level) = 0;                          // Power level for the antenna  
    virtual void sleep() = 0;                                                       // Put the transceiver in idle or power-down mode when not actively transmitting.

};