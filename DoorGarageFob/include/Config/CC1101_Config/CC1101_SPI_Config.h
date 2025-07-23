#pragma once

#include <Arduino.h>

/// @brief Masks for SPI address byte (bits 7–6 for access type, bits 5–0 for address) 
/// The CC1101 transceiver uses a specific SPI addressing scheme to access its registers, FIFOs, and strobe commands. According to the CC1101 datasheet, the first byte sent during an SPI transaction is the address byte, which includes:
/// Bits 7–6: Access type (read, write, burst).
/// Bits 5–0: Register address (0x00–0x3F).
namespace SPI_MASK { 
    namespace Masks { 
        constexpr uint8_t WriteSingle = 0x7F;                   // Clear bit 7 for single-byte write (0b0XXXXXXX) 
        constexpr uint8_t ReadSingle = 0x80;                   // Set bit 7 for single-byte read (0b1XXXXXXX) 
        constexpr uint8_t writeBurstRegister = 0x40;         // Set bit 6 for burst write (0b01XXXXXX) 
        constexpr uint8_t readBurstRegister = 0xC0;         // Set bits 7–6 for burst read (0b11XXXXXX) 
        constexpr uint8_t AddressMask = 0x3F;               // Mask for 6-bit address (0x00–0x3F) 
        constexpr uint8_t DummyByte = 0x00;                 // To read the actual register value, the master must send another byte (typically 0x00) to keep the clock ticking.
        constexpr uint8_t ChipState = 0x0F;                    // Apply after right shift  >>4 Chip states bit 7-4 to read chip state    
        constexpr uint8_t FIFObytes = 0x0F;                    // Mask to extract FIFO bytes (bits 3–0)
   }
}