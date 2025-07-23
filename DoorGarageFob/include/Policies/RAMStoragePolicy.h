#pragma once

#include <stdint.h>

/**
 * @brief Define a specialize behavior to read the Config from the RAM memory
 * 
 */
struct RAMStoragePolicy
{
    /**
     * @brief This is how RAM access should be handled in AVR or any microcontroller without segmented memory constraints.
     * 
     * @param ptr - Accepts a pointer to RAM address from where we want to read
     * @return uint8_t - Derefeance the ptr to get the value store in memory
     */
    static uint8_t read(const uint8_t* ptr)
    {
        return *ptr;                                        // Returns the value stored in the address by dereference the pointer
    }

    /**
     * @brief Overload for bool to handle regSettings.verify.
     * 
     * @param ptr 
     * @return true 
     * @return  bool to match the expected type in applyRegisterConfig_CC1101.
     */
    static bool read(const bool* ptr)
    {
        return *ptr;
     } 
};