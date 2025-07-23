#include "Config/TransceiverConfig.h"



TransceiverConfig::TransceiverConfig(uint32_t transmissionFreqBand_Hz, ModulationScheme modulation, OutputPowerLevels powerLevel):
_transmission_frequency_Hz(transmissionFreqBand_Hz),
_modulationScheme(modulation),
_powerLevel(powerLevel)
{
}

uint32_t TransceiverConfig::getFrequencyHz() const
{
    return _transmission_frequency_Hz;
}

ModulationScheme TransceiverConfig::getModulationScheme() const
{
    return _modulationScheme;
}

OutputPowerLevels TransceiverConfig::getPowerLevel() const
{
    return _powerLevel;
}

/// @brief 
/// @return 
uint8_t TransceiverConfig::getPATableIndex() const
{
    switch (_powerLevel)
    {
        case OutputPowerLevels::LOW_POWER : return PATABLE_LOW_INDEX; break;
        case OutputPowerLevels::MEDIUM_POWER : return PATABLE_MEDIUM_INDEX; break;
        case OutputPowerLevels::HIGH_POWER : return PATABLE_HIGH_INDEX; break; 
        default: return 0;  break;                                                                                                  // Fallbak to highest power
    }
}
