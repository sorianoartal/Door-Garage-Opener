#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>

/**
 * @brief Define the behavior for read from the PROGMEM 
 * 
 */
struct PROGMEMStoragePolicy
{
    static uint8_t read(const uint8_t* ptr)
    {
        return pgm_read_byte(ptr);                  // Return the value store in the address using the correct macro for AVR platforms to safely dereference data in flash. 
    }

    // overload read to handle verify
    static bool read(const bool* ptr) { return pgm_read_byte(reinterpret_cast<const uint8_t*>(ptr)) != 0; }
};
