#pragma once

#include <array>
#include "Config/CC1101_Config/RegisterSettings.h"
#include "Config/CC1101_Config/CC1101.h"
#include "Config/Constants.h"
#include <avr/pgmspace.h>

/**
 * @brief to organize and encapsulate the configuration settings for the CC1101 transceiver module configured for 315 MHz OOK (On-Off Keying) communication on an ATmega328P. 
 *  Namespaces in C++ are a mechanism to group related identifiers (like variables, functions, or types) under a named scope, preventing naming conflicts, improving code readability, and making the code more modular
 *  - By using the configuration in a name space , we can reuse it across different parts of the program  or other projects
 */
namespace Config_315MHz_OOK
{
    
    constexpr std::array<RegisterSettings,NUM_REG_TO_CONFIG_CC1101> setting_Regs =
    {
        {
            { CC1101::Address::IOCFG0 ,      CC1101::Value::IOCFG0 ,     VERIFY},               // GDO0 output: High-Z
            { CC1101::Address::FIFOTHR ,     CC1101::Value::FIFOTHR,     VERIFY},              // RX/TX FIFO thresholds
            { CC1101::Address::PKTLEN,       CC1101::Value::PKTLEN,      SKIP_VERIFY},      // Max packet length (not used in async)
            { CC1101::Address::PKTCTRL0,    CC1101::Value::PKTCTRL0,   VERIFY},              // No whitening, no packet format
            { CC1101::Address::FSCTRL1,      CC1101::Value::FSCTRL1,     VERIFY},             // IF frequency
            { CC1101::Address::FSCTRL0,      CC1101::Value::FSCTRL0,     VERIFY},             // Frequency offset
            { CC1101::Address::FREQ2,         CC1101::Value::FREQ2,         VERIFY},            // Frequency high byte (315 MHz)
            { CC1101::Address::FREQ1,         CC1101::Value::FREQ1,         VERIFY},            // Frequency middle byte
            { CC1101::Address::FREQ0,         CC1101::Value::FREQ0,         VERIFY},            // Frequency low byte
            { CC1101::Address::MDMCFG4,   CC1101::Value::MDMCFG4,   VERIFY},           // Modem config (data rate)
            { CC1101::Address::MDMCFG3,   CC1101::Value::MDMCFG3,   VERIFY},           // Modem config (data rate)
            { CC1101::Address::MDMCFG2,   CC1101::Value::MDMCFG2,   VERIFY},            // OOK modulation
            { CC1101::Address::MDMCFG1,   CC1101::Value::MDMCFG1,   VERIFY},            // Set minimum preamble for asynchronous mode  
            { CC1101::Address::DEVIATN,      CC1101::Value::DEVIATN,      SKIP_VERIFY},    // Not used for OOK
            { CC1101::Address::MCSM1,       CC1101::Value::MCSM1,        VERIFY},           // State machine
            { CC1101::Address::MCSM0,       CC1101::Value::MCSM0,        VERIFY},           // Auto-calibrate, TX after IDLE
            { CC1101::Address::WORCTRL,    CC1101::Value::WORCTRL,     VERIFY},          // Wake-on-radio control
            { CC1101::Address::FREND1,       CC1101::Value::FREND1,        VERIFY},          // Front end RX config
            { CC1101::Address::FREND0,       CC1101::Value::FREND0,        VERIFY},          // Front end TX config (OOK)
            { CC1101::Address::FSCAL3,        CC1101::Value::FSCAL3,          SKIP_VERIFY},          // Frequency synthesizer calibration
            { CC1101::Address::FSCAL2,        CC1101::Value::FSCAL2,          SKIP_VERIFY },  // Frequency synthesizer calibration
            { CC1101::Address::FSCAL1,        CC1101::Value::FSCAL1,          SKIP_VERIFY},   // Frequency synthesizer calibration
            { CC1101::Address::FSCAL0,        CC1101::Value::FSCAL0,          SKIP_VERIFY}   // Frequency synthesizer calibration
      }
   };

    // PROGMEM configuration for 315 MHz OOK
    // Keep constant data in flash (program) memory only, instead of copying it to SRAM when the program starts. 
   static const RegisterSettings setting_Regs_pgm[] PROGMEM =
   {
            { CC1101::Address::IOCFG0 ,      CC1101::Value::IOCFG0 ,     VERIFY},               // GDO0 output: High-Z
            { CC1101::Address::FIFOTHR ,     CC1101::Value::FIFOTHR,     VERIFY},              // RX/TX FIFO thresholds
            { CC1101::Address::PKTLEN,       CC1101::Value::PKTLEN,      SKIP_VERIFY},      // Max packet length (not used in async)
            { CC1101::Address::PKTCTRL0,    CC1101::Value::PKTCTRL0,   VERIFY},              // No whitening, no packet format
            { CC1101::Address::FSCTRL1,      CC1101::Value::FSCTRL1,     VERIFY},             // IF frequency
            { CC1101::Address::FSCTRL0,      CC1101::Value::FSCTRL0,     VERIFY},             // Frequency offset
            { CC1101::Address::FREQ2,         CC1101::Value::FREQ2,         VERIFY},            // Frequency high byte (315 MHz)
            { CC1101::Address::FREQ1,         CC1101::Value::FREQ1,         VERIFY},            // Frequency middle byte
            { CC1101::Address::FREQ0,         CC1101::Value::FREQ0,         VERIFY},            // Frequency low byte
            { CC1101::Address::MDMCFG4,   CC1101::Value::MDMCFG4,   VERIFY},           // Modem config (data rate)
            { CC1101::Address::MDMCFG3,   CC1101::Value::MDMCFG3,   VERIFY},           // Modem config (data rate)
            { CC1101::Address::MDMCFG2,   CC1101::Value::MDMCFG2,   VERIFY},            // OOK modulation
            { CC1101::Address::MDMCFG1,   CC1101::Value::MDMCFG1,   VERIFY},            // Set minimum preamble for asynchronous mode  
            { CC1101::Address::DEVIATN,      CC1101::Value::DEVIATN,      SKIP_VERIFY},    // Not used for OOK
            { CC1101::Address::MCSM1,       CC1101::Value::MCSM1,        VERIFY},           // State machine
            { CC1101::Address::MCSM0,       CC1101::Value::MCSM0,        VERIFY},           // Auto-calibrate, TX after IDLE
            { CC1101::Address::WORCTRL,    CC1101::Value::WORCTRL,     VERIFY},          // Wake-on-radio control
            { CC1101::Address::FREND1,       CC1101::Value::FREND1,        VERIFY},          // Front end RX config
            { CC1101::Address::FREND0,       CC1101::Value::FREND0,        VERIFY},          // Front end TX config (OOK)
            { CC1101::Address::FSCAL3,        CC1101::Value::FSCAL3,          SKIP_VERIFY},          // Frequency synthesizer calibration
            { CC1101::Address::FSCAL2,        CC1101::Value::FSCAL2,          SKIP_VERIFY },  // Frequency synthesizer calibration
            { CC1101::Address::FSCAL1,        CC1101::Value::FSCAL1,          SKIP_VERIFY},   // Frequency synthesizer calibration
            { CC1101::Address::FSCAL0,        CC1101::Value::FSCAL0,          SKIP_VERIFY}   // Frequency synthesizer calibration     
   }; 
}
