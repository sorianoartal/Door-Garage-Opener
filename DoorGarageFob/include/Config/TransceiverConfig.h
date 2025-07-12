#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "Constants.h"


enum class ModulationScheme
{
    OOK,                                                      // ON/OFF Frequency Shift Keying 
    FSK4,                                                      // Quaternary Frequency Shift Keying 
    FSK2,                                                      // Binary Frequency Shift Keying
    GFSK,                                                     // Gaussian shaped Frequency Shift Keying
    MSK                                                      // Minimum Shift Keying   
};

// The RF output power level from the device datasheet
enum class  OutputPowerLevels
{
    HIGH_POWER,
    MEDIUM_POWER,
    LOW_POWER
};

/**
 * @brief Configuration use for CC1101 RF Module
 * 
 */
struct TransceiverConfig
{
    public:
    
    // Parametrize constructor
    TransceiverConfig(uint32_t transmissionFreqBand_Hz = FREQ_315MHZ_BAND, ModulationScheme modulation = ModulationScheme::OOK, OutputPowerLevels powerLevel = OutputPowerLevels::HIGH_POWER);

    // Destructor
    ~TransceiverConfig() = default;

    // Getters so the Transceiver can read the configuration parameters and we avoid the possible modification
    uint32_t getFrequencyHz() const;
    ModulationScheme getModulationScheme() const;
    OutputPowerLevels getPowerLevel() const;
      uint8_t getPATableIndex() const;                                                                  //                    

    private:

    uint32_t _transmission_frequency_Hz;                // Tran
    ModulationScheme _modulationScheme;
    OutputPowerLevels _powerLevel;
};
