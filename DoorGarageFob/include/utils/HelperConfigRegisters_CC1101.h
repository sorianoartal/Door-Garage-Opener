#pragma once

#include<array>

#include "Debugging/Logging.h"
#include "Config/CC1101_Config/RegisterSettings.h"      // Struct for register configuration
#include "Transciever/CC1101_Transceiver.h"
#include "avr_algorithms.hpp"                   // for_each utility function

/**
 * @brief Configures the CC1101 Register using a storage Policy Strategy
 * 
 * @tparam StoragePolicy Define from where we need to read the configuration registers (e.g., RAMStoragePolicy, PROGMEMStoragePolicy)
 * @tparam Write Type of the write register function (bool(uint8_t, uint8_t))
 * @tparam Read Type of the read register function (uint8_t(uint8_t))
 * @param config Pointer to the array of RegisterSettings
 * @param N Size of the configuration array
 * @param writeRegister Function to write a register to the CC1101
 * @param readRegister Function to read from a register of the CC1101
 * @return true if all write operations succeed and verification passes, false otherwise
 */
template<typename StoragePolicy, typename Write, typename Read>
bool applyRegisterConfig_CC1101(const RegisterSettings* config, size_t N, Write&& writeRegister, Read&& readRegister)
{
    auto writeAndVerifyRegister = [&writeRegister, &readRegister](const RegisterSettings& regSettings) {
        // Step 1: Use the correct Policy to read configuration from memory
        uint8_t address = StoragePolicy::read(&regSettings.reg);
        uint8_t value = StoragePolicy::read(&regSettings.reg_value);
        bool verify = StoragePolicy::read(&regSettings.verify);

        // Log every successful register write (for trace)
        #ifdef LOG_VERBOSE
        LOG("---- Register Write OK ----");
        LOG("Wrote Register:");
        LOG_PAIR_HEX("- Address:", address);
        LOG_PAIR_HEX("- Value:", value);
        #endif

        // Step 2: Write configuration and check for operation success
        if (!writeRegister(address, value)) {
            // Log configuration error
            LOG("---- Register Config Failure ----");
            LOG("Error: Writing operation failed for Register configuration: ");
            LOG_PAIR_HEX("- Address: ", address);
            LOG_PAIR_HEX("- Value:", value);
            return false;
        }

        // Step 3: Verify if the register was written correctly
        if (verify) {
            if (auto writeValue = readRegister(address); writeValue != value) {
                // Log verification error
                LOG("---- Register Config Failure ----");
                LOG("Error: Fail when trying to read CC1101 register:");
                LOG_PAIR_HEX("- Address: ", address);
                LOG_PAIR_HEX("- Expected: ", value);
                LOG_PAIR_HEX("- Readback: ", writeValue);
                return false;
            }
        }
        return true; // Success
    };

     avr_algorithms::for_each_element(config, N, writeAndVerifyRegister); // Use pointer-based for_each
     return true; // All registers written and verified successfully
}

