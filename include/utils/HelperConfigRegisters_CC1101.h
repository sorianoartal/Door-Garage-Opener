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
        String registerLabel = String(CC1101::registerName(address)) + String(F(" (")) + formatHex8(address) + ')';

        #ifdef LOG_VERBOSE
        String beginMsg = String(F("[CC1101][CFG] Apply ")) + registerLabel + String(F(" = ")) + formatHex8(value);
        if (verify) {
            beginMsg += F(" [verify]");
        }
        LOG_DYNAMIC(beginMsg);
        #endif

        // Step 2: Write configuration and check for operation success
        if (!writeRegister(address, value)) {
            LOG_DYNAMIC(String(F("[CC1101][CFG][ERROR] Write failed for ")) + registerLabel +
                        String(F(", target value ")) + formatHex8(value));
            return false;
        }

        // Step 3: Verify if the register was written correctly
        if (verify) {
            uint8_t attempts = 0;
            bool verified = false;

            avr_algorithms::repeat_withExitCondition(3, [&]() {
                delayMicroseconds(50);
                uint8_t writeValue = readRegister(address);
                if (writeValue == value) {
                    verified = true;
                    return false;
                }

                LOG_DYNAMIC(String(F("[CC1101][CFG][VERIFY-RETRY ")) + String(attempts + 1) +
                            F("/3] ") + registerLabel +
                            String(F(": expected ")) + formatHex8(value) +
                            String(F(", read ")) + formatHex8(writeValue));
                attempts++;
                return true;
            });

            if (!verified) {
                LOG_DYNAMIC(String(F("[CC1101][CFG][ERROR] Verify failed for ")) + registerLabel +
                            String(F(" after 3 attempts, expected ")) + formatHex8(value));
                return false;
            }
        }
        return true; // Success
    };

     return avr_algorithms::for_each_until(config, N, writeAndVerifyRegister);
}

