#pragma once
#include <stdint.h>

/**
 * @brief Groups all CC1101 registers addresses , values and Strobe commands in a single logical place 
 */
namespace CC1101
{
   /// @brief Address space for the CC1101 transceiver registers and configuration settings.
   /// @details This namespace contains constants for the addresses of various registers 
   namespace Address
   {
            constexpr uint8_t IOCFG0     = 0x02;         // GDO0 output: Output Pin Configuration Pag. 71 datasheet
            constexpr uint8_t FIFOTHR    = 0x03;         // 0x03: FIFOTHR – RX FIFO and TX FIFO Thresholds pag 72
            constexpr uint8_t PKTLEN     = 0x06;         // Max packet length (not used in async)
            constexpr uint8_t PKTCTRL0  = 0x08;         // 0x08: PKTCTRL0 – Packet Automation Control
            constexpr uint8_t FSCTRL1    = 0x0B;         // FSCTRL1 – Frequency Synthesizer Control pag75 -> The default value gives an IF frequency of 381kHz, assuming a 26.0 MHz crystal.
            constexpr uint8_t FSCTRL0    = 0x0C;         // FSCTRL0 – Frequency Synthesizer Control -> Frequency offset added to the base frequency before being used by the frequency synthesizer. (2s-complement).
            constexpr uint8_t FREQ2       = 0x0D;        // FREQ2 – Frequency Control Word, High Byte
            constexpr uint8_t FREQ1       = 0x0E;         // FREQ1 – Frequency Control Word, Middle Byte
            constexpr uint8_t FREQ0       = 0x0F;         // Frequency Control Word, Low Byte = 315 MHz
            constexpr uint8_t MDMCFG4 = 0x10;        // MDMCFG4 – Modem Configuration. Pag 76
            constexpr uint8_t MDMCFG3 = 0x11;        // MDMCFG3 – Modem Configuration -Mantissa refers to the fractional part of a number in its decimal equivalent form. For example, the mantissa of the number 3.75 is 75
            constexpr uint8_t MDMCFG2 = 0x12;        // Modulation: OOK (bits 6:4 = 011)
            constexpr uint8_t MDMCFG1 = 0x13;        // 
            constexpr uint8_t DEVIATN    = 0x15;        // Not used for OOK
            constexpr uint8_t FREND0     = 0x22;        // FREND0 – Front End TX Configuration -In OOK/ASK mode, this selects the PATABLE index to use when transmitting a ‘1’. PATABLE index zero is used in OOK/ASK when transmitting a ‘0’.
            constexpr uint8_t FREND1     = 0x21;        // FREND1 – Front End RX Configuration 
            constexpr uint8_t MCSM0     = 0x18;        // MCSM0– Main Radio Control State Machine Configuration --> Auto-calibrate, TX mode after IDLE
            constexpr uint8_t MCSM1     = 0x17;        // MCSM1– Main Radio Control State Machine Configuration  
            constexpr uint8_t WORCTRL  = 0x20;        // WORCTRL – Wake On Radio Control 
            constexpr uint8_t FSCAL3      = 0x23;        // FSCAL3 – Frequency Synthesizer Calibration
            constexpr uint8_t FSCAL2      = 0x24;        // FSCAL2 – Frequency Synthesizer Calibration
            constexpr uint8_t FSCAL1      = 0x25;        // FSCAL1 – Frequency Synthesizer Calibration
            constexpr uint8_t FSCAL0      = 0x26;        // FSCAL0 – Frequency Synthesizer Calibration
            constexpr uint8_t PATABLE     = 0x3E;        // Power config register
            constexpr uint8_t PARTNUM  = 0x30;        // Chip ID - Part number for CC1101
            constexpr uint8_t VERSION    = 0x31;        // Chip version number. Subject to change without notice.
            constexpr uint8_t MARCSTATE = 0X35;      // Control state machine state
   }

    /// @brief Values for the CC1101 transceiver registers and configuration settings.
    /// @details This namespace contains constants for the values to be written to various registers and configuration settings for the CC1101 transceiver.
   namespace Value
   {
        constexpr uint8_t IOCFG0        = 0x2E;        // GDO0 pin output behavior : High-Z (manual control)
        constexpr uint8_t FIFOTHR       = 0x07;        // Bits 3–0: FIFO_THR = 0x07 (default, 33 bytes TX, 32 bytes RX threshold).  
        constexpr uint8_t PKTLEN        = 0xFF;        // Sets maximum packet length. In async mode (PKTCTRL0 = 0x32), packet length is ignored as no packet structure is used. Setting to max (0xFF) is safe and standard. Matches datasheet section 17.2.
        constexpr uint8_t PKTCTRL0     = 0x32;       // Configures packet control (whitening, format, CRC). -> Conf : Disables packet handling, whitening, and CRC, enabling serial data via GDO0. Matches datasheet section 17.2 and your comment (“No whitening, no packet format”).
        constexpr uint8_t FSCTRL1       = 0x06;       // Sets intermediate frequency (IF). IF frequency = FREQ_IF × f_XOSC / 2^10. For 26 MHz crystal, 0x06 × 26 MHz / 1024 ≈ 152.3 kHz. Suitable for 315 MHz OOK with typical channel bandwidth (see MDMCFG4). Matches datasheet section 14.2.
        constexpr uint8_t FSCTRL0       = 0x00;       // Frequency offset compensation. Assumes crystal accuracy is sufficient for 315 MHz. Acceptable for stable crystals (e.g., 26 MHz ±10 ppm). If frequency drift occurs, adjust based on calibration (datasheet section 14.3).
        constexpr uint8_t FREQ2          = 0x0C;      // Sets carrier frequency (315 MHz). Frequency = FREQ × f_XOSC / 2^16. Very close to 315 MHz (error ~12 kHz, <0.004%). Within typical receiver tolerance for OOK (e.g., ±100 kHz). Matches datasheet section 14.1.
        constexpr uint8_t FREQ1          = 0x1C;
        constexpr uint8_t FREQ0          = 0xEC;       // = 315 MHz
        constexpr uint8_t MDMCFG4    = 0xF5;       // Modem configuration (channel bandwidth, data rate exponent). Bandwidth = f_XOSC / (8 × (4 + CHANBW_M) × 2^CHANBW_E). 81.25 kHz BW is narrow but suitable for low-rate OOK (see MDMCFG3). Matches datasheet section 13.5.
        constexpr uint8_t MDMCFG3    = 0x83;       // Sets data rate mantissa.  Data rate = (256 + DRATE_M) × 2^DRATE_E × f_XOSC / 2^28. 1.2 kbps is low but typical for OOK (e.g., remote controls). Matches datasheet section 13.5.
        constexpr uint8_t MDMCFG2    = 0x30;       // Modulation: OOK (bits 6:4 = 011) OOK modulation and no sync word are perfect for async OOK. Matches datasheet section 13.5
        constexpr uint8_t MDMCFG1    = 0x02;       // Disable preamble or asynchronous mode. No preamble is ideal for async OOK (PKTCTRL0 = 0x32). CHAN_SPC_E = 2 is safe (channel spacing irrelevant for single channel). Matches datasheet section 13.5
        constexpr uint8_t DEVIATN       = 0x15;       // Frequency deviation for FSK/PSK (not used in OOK).Not used for OOK
        constexpr uint8_t FREND0        = 0x11;       // Front-end TX configuration (PA table index).
        constexpr uint8_t FREND1        = 0xB6;       // Front-end RX configuration (LNA, mixer). Optimized for RX, but irrelevant for TX-only OOK. Default-like value is safe. Matches datasheet section 10.5.
        constexpr uint8_t MCSM0        = 0x18;       // State machine configuration (calibration, pin control). Auto-calibration on IDLE-to-TX transition ensures frequency accuracy for 315 MHz. Matches datasheet section 19.2 and your comment (“Auto-calibrate, TX mode after IDLE”).
        constexpr uint8_t MCSM1        = 0x30;       // Main radio control state machine (next state after RX/TX). Suitable for async TX, ensures IDLE state after transmission. CCA_MODE is irrelevant for TX-only OOK. Matches datasheet section 19.2.
        constexpr uint8_t WORCTRL     = 0xFB;       // Wake-on-radio control. WOR is unused in async OOK, so disabling RC oscillator (RC_PD = 1) is correct. Default settings are safe. Matches datasheet section 19.3.
        constexpr uint8_t FSCAL3         = 0xE9;       // Frequency synthesizer calibration.  FSCAL3 = 0xE9: Calibration coefficients.
        constexpr uint8_t FSCAL2         = 0x2A;       // FSCAL2 = 0x2A: VCO settings.  
        constexpr uint8_t FSCAL1         = 0x00;       // FSCAL1 = 0x00: VCO current.  
        constexpr uint8_t FSCAL0         = 0x1F;       // FSCAL0 = 0x1F: Calibration loop.  
   }

   /// @brief Strobe commands from Table 42, page 62, CC1101 datasheet (TI)
   /// @details These commands are used to control the CC1101 transceiver's state machine.     
   namespace Strobes
   {     
      /// @brief Strobe commands for the CC1101 transceiver.
      enum class Command : uint8_t
      {
            SRES = 0x30,            // Reset chip
            SFSTXON = 0x31,         // Enable and calibrate frequency synthesizer
            SXOFF = 0x32,           // Turn off crystal oscillator
            SCAL = 0x33,            // Calibrate frequency synthesizer and turn it off
            SRX = 0x34,             // Enable RX
            STX = 0x35,             // Enable TX
            SIDLE = 0x36,           // Exit RX/TX, turn off frequency synthesizer
            SWOR = 0x38,            // Start automatic RX polling (Wake-on-Radio)
            SPWD = 0x39,            // Enter power down mode
            SFRX = 0x3A,            // Flush RX FIFO
            SFTX = 0x3B,            // Flush TX FIFO
            SWORRST = 0x3C,         // Reset real-time clock
            SNOP = 0x3D             // No operation
      };
   }
}
