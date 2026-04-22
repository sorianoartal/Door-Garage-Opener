#include "Config/TransceiverConfig.h"



uint8_t outputPowerLevelToOOKLogicOnePowerByte(OutputPowerLevel powerLevel)
{
    switch (powerLevel)
    {
        case OutputPowerLevel::LOW_POWER: return LOW_BAND_OOK_LOGIC_ONE_POWER_LOW;
        case OutputPowerLevel::MEDIUM_POWER: return LOW_BAND_OOK_LOGIC_ONE_POWER_MEDIUM;
        case OutputPowerLevel::HIGH_POWER: return LOW_BAND_OOK_LOGIC_ONE_POWER_HIGH;
        default: return LOW_BAND_OOK_LOGIC_ONE_POWER_HIGH;
    }
}

TransceiverConfig::TransceiverConfig(uint32_t transmissionFreqBand_Hz, ModulationScheme modulation, OutputPowerLevel powerLevel):
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

OutputPowerLevel TransceiverConfig::getPowerLevel() const
{
    return _powerLevel;
}

/// @brief Returns the low-band PATABLE byte to use for OOK logic '1'.
/// @return PATABLE entry value for the selected output power level.
uint8_t TransceiverConfig::getOOKLogicOnePowerByte() const
{
    return outputPowerLevelToOOKLogicOnePowerByte(_powerLevel);
}
