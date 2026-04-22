#pragma once

#include <stdint.h>
#include "Config/TransceiverConfig.h"

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
    virtual void setPowerLevel(OutputPowerLevel powerLevel) = 0;      // Update the OOK logic '1' power level used in PATABLE.
    virtual void sleep() = 0;                                                       // Put the transceiver in idle or power-down mode when not actively transmitting.

};
