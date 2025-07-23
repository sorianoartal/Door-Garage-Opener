#pragma once

#include <stdint.h>

// -------------------------------------------------------------------------
// RegisterSettings structure
// -------------------------------------------------------------------------    

/**
 * @brief This structure is used to store settings for configuring a specific register in the CC1101 transceiver.
 *  @note It includes the register address, the value to write to that register, and a flag to indicate whether the register is readable.
 *  @note The 'verify' flag is useful for registers that can be read back to confirm the write operation was successful.
 *  @struct RegisterSettings 
 */
struct RegisterSettings
{
    uint8_t reg;                // Address of the register to config
    uint8_t reg_value;                    // Store the value we want to write in the reg
    bool verify;                            //  Set to true if the register is readable, so it can be verify after write 
};
