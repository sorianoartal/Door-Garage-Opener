#include "Transciever/CC1101_Transceiver.h"


/**
 * @brief Transceiver constructor
 * @note: This constructor initializes the CC1101 transceiver with a given SPIBus instance and configuration.
 * @param spi - SPIBus instance for low-level communication
 */
Transceiver::Transceiver(  SPIBus& spi, const TransceiverConfig& config):
_spi(spi),
_transceiver_config(config)
{}

/**
 * @brief Transceiver initialization logic:
 * - Initialize the SPI interface 
 * - Reset the chip to ensure a known state
 * - Apply register configuration
 * - Configure PATABLE
 * - Verify key registers (like PARTNUM or version ID).
 */
bool Transceiver::begin()
{
   #ifdef LOG_VERBOSE
     LOG("Transceiver::begin() - Initializing CC1101 Transceiver");
     printDots(5, 750); // Print 5 dots with a 500 ms delay between each dot
   #endif 
    
    // Step1: Initialize the SPI
    _spi.begin();

    #ifdef LOG_VERBOSE  
    LOG_NEW_LINE("SPIBus::begin() called.");
    #endif


    // Step2: Reset Transceiver
    reset();

   #ifdef LOG_VERBOSE  
    LOG_NEW_LINE("Transceiver reset complete.");
    #endif

    // Step3: Apply Register Configuration

    // Wrap Read/Write operation in a Lambda to pass it to the Helper
    auto writeLambda = [this](uint8_t addr, uint8_t val) {
        return _spi.writeRegister(addr, val);
    };

    auto readLambda = [this](uint8_t addr) -> uint8_t {
        return _spi.readRegister(addr).value;  // Ignore status for config verification
    };

    applyRegisterConfig_CC1101<RAMStoragePolicy>(
        Config_315MHz_OOK::setting_Regs.data(),
        Config_315MHz_OOK::setting_Regs.size(),
        writeLambda,
        readLambda
    );

    // Step4: Configure PATABLE
    configurePATable(_transceiver_config.getPATableIndex());

    // Step5 : Check if CC1101 is responsive after configuration verifying PARTNUM & VERSION  
    bool success = false;
    avr_algorithms::repeat_withExitCondition(3,[&](){
        // PARTNUM == 0x00, it indicates the CC1101 is powered, connected, and responsive to SPI
        if (readRegister(CC1101::Address::PARTNUM).value != 0x00)
        {
            LOG_NEW_LINE("------ SPI communication Error   ------");
            LOG_NEW_LINE(" Error: apply configuration of CC1101 failed in Transceiver::begin() ");
            LOG_NEW_LINE(" Fail to read PARTNUM ");
            return true;    // Retry
        }
  
         // VERSION register at 0x30 address after reset should read 0x14 HEX  
        if (readRegister(CC1101::Address::VERSION).value != 0x14)
        {
            LOG_NEW_LINE("------ SPI communication Error   ------");
            LOG_NEW_LINE(" Error: apply configuration of CC1101 failed in Transceiver::begin() ");
            LOG_NEW_LINE("Fail to read VERSION");
            return true;    // Retry
        }     
        
        success = true;
        return false;           // Break repeat()
   });

   if (!success)
   {
        LOG_NEW_LINE("Error: CC1101 initialization failed");
        return false;
   }   

   LOG("\n\n");
   return true;
}


 /// @brief The CC1101 operates in distinct states (e.g., IDLE, TX, RX, SLEEP), controlled by strobe commands (per datasheet, section 13).
 /// To transmit data the CC1101 module must be in TX mode, to do so we need to activated by sending STX strobe.
 /// without us enable the TX mode data won't be write to the FIFO
 /// What this function does :
 ///    - Checks MARCSTATE (0x01 = IDLE) before STX.
///     - Issues SIDLE if not in IDLE.
///     - Sends STX via strobeCommand.
///     - Verifies MARCSTATE = 0x13 (TX) after STX.
///     - Logs errors for debugging.
 void Transceiver::enableTransmitMode()
{
    using Strobe = CC1101::Strobes::Command;
    bool success = false;
    String error = "Transceiver::enableTransmitMode(): TX mode Active Successfully...";

    // Check if already in IDLE
    if (readRegister(CC1101::Address::MARCSTATE).value != 0x01) {
      avr_algorithms::repeat_withExitCondition(3, [&]() {
            if (!strobeCommand(Strobe::SIDLE)) {
                error = "Error: Failed to enter IDLE mode";
                return true; // Retry
            }
            delayMicroseconds(10); // Wait for transition
            success = true;
            return false; // Exit repeat
        });
        if (!success) {
            LOG_DYNAMIC(error);
            return;
        }
    }

   // Transition to TX
   success = false;
   avr_algorithms::repeat_withExitCondition(3, [&]() {
        if (!strobeCommand(Strobe::STX)) {
            error = "Error: Failed to enter TX mode";
            return true; // Retry
        }
        if (readRegister(CC1101::Address::MARCSTATE).value != 0x13) {
            error = "Error: Failed to confirm TX mode (MARCSTATE != 0x13)";
            return true; // Retry
        }
        success = true;
        return false; // Exit repeat
    });

    LOG_DYNAMIC(error);
    LOG("\n\n");
}

/**
 * @brief  Implement the Manual Power-on reset via Spi to ensure the Chip is in a Know state(IDLE) before configuration
 *   This method execute the Following sequence as specify on the datasheet
 *     Step1 :  Ensure SCK = 1 and MOSI =0.
 *     Step2 :  Pull CSn LOW for at least 10 µs.
*      Step3 :  Pull CSn HIGH for at least 40 µs.
*      Step4 :  Pull CSn LOW again to start SPI transaction.
*      Step5 :  Wait for MISO to go low  
*      Step6 :  Send SRES (0x30).
*      Step7 :  Wait for the chip to stabilize (typically 10 ms, as the crystal oscillator restarts).  
*      Step8 : Verify if PARTNUM(address 0x30) == 0x00   after reset
 */
void Transceiver::reset()
{
    // Retry attempts for reset sequence
    // This will attempt the reset sequence up to 3 times, exiting early if successful.
    uint8_t attempt = 0; 


    /*  
        // The following code is commented out as it is a legacy implementation.
        // The new implementation uses the SPIBus class for better abstraction and error handling.  
     for (int attempts = 0; attempts < 3; attempts++) {
        // 1. Pull CSn LOW for >10us
        digitalWrite(CSN_PIN, LOW);
        delayMicroseconds(10);

        // 2. Pull CSn HIGH for >40us
        digitalWrite(CSN_PIN, HIGH);
        delayMicroseconds(40);

        // 3. Pull CSn LOW to start SPI
        digitalWrite(CSN_PIN, LOW);

        // 4. Wait for MISO to go LOW (chip ready)
        unsigned long start = millis();
        while (digitalRead(MISO) == HIGH) {
            if (millis() - start > 100) {
                LOG_NEW_LINE("Timeout waiting for MISO LOW before SRES");
                break;
            }
        }

        // 5. Send SRES (while CSn is still LOW!)
        SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
        SPI.transfer(0x30); // SRES
        SPI.endTransaction();

        // 6. Wait for MISO to go LOW again (reset finished)
        start = millis();
        while (digitalRead(MISO) == HIGH) {
            if (millis() - start > 100) {
                LOG_NEW_LINE("Timeout waiting for MISO LOW after SRES");
                break;
            }
        }

        // 7. Pull CSn HIGH (end transaction)
        digitalWrite(CSN_PIN, HIGH);

        // 8. Wait for crystal to stabilize (typically 10 ms)
        delay(10);

        // 9. Verify PARTNUM register
        if (verifyChipId()) {
            LOG_NEW_LINE("CC1101 reset successful");
            return;
        }
    }

    LOG_NEW_LINE("Error: CC1101 reset failed after 3 attempts");
    */

    // The following code is a more robust implementation using the SPIBus class for better abstraction and error handling.
    
    // Retry mechanism for reset sequence
    // This will attempt the reset sequence up to 3 times, exiting early if successful.
    // The reset sequence is wrapped in a lambda function to allow retries with exit conditions.
    // The lambda captures the necessary state and performs the reset steps, logging errors as needed.
    avr_algorithms::repeat_withExitCondition(3, [&]() {
        #ifdef LOG_VERBOSE          
        LOG("Transceiver::reset() - Attempting reset sequence");
        printDots(3, 500); // Print 3 dots with a 500 ms delay between each dot
        #endif
        
        LOG_NEW_LINE("Transceiver::reset() - Starting reset sequence");
        String msg = "attempt: " + String(attempt + 1);
        LOG_DYNAMIC(msg);

        // Step1: Pull CSn LOW for at least 10 µs
        _spi.selectDevice();
        delayMicroseconds(10);

        // Step2: Pull CSn HIGH for at least 40 µs
        _spi.deselectDevice();
        delayMicroseconds(40);

        // Step3: Pull CSn LOW again to start SPI transaction
        _spi.selectDevice();

        // Step4: Wait for MISO to go low (indicating chip is ready)
        unsigned long start = millis();
        // Polling MISO until it goes LOW or timeout after 100 ms
        while (digitalRead(MISO) == HIGH) {
            if (millis() - start > 100) {
                LOG_NEW_LINE("Timeout waiting for MISO LOW before SRES");
                break; // Exit loop on timeout
            }
        }

        // Step5: Send SRES command (0x30)
        if (!strobeCommand(CC1101::Strobes::Command::SRES)) {
            LOG_NEW_LINE("Error: Failed to send SRES command");
            return true; // Retry
        }

        // Step6: Wait for MISO to go low again (indicating reset finished)
        start = millis();
        while (digitalRead(MISO) == HIGH) {
            if (millis() - start > 100) {
                LOG_NEW_LINE("Timeout waiting for MISO LOW after SRES");
                break; // Exit loop on timeout
            }
        }
        // Step7: Wait for the chip to stabilize (typically 10 ms)
        delay(10);

        // Step8: Verify PARTNUM register
        if (verifyChipId()) {
            LOG_NEW_LINE("CC1101 reset successful");
            return false; // Exit on success
        }
    }
    );

    LOG_NEW_LINE("Error: CC1101 reset failed after 3 attempts");
    LOG("\n\n");
}


/// @brief Check  for the expected value after reset in the PARTNUM register
/// @return True if PARTNUM is 0x00  
bool Transceiver::verifyChipId()
{
    if(ReadResult result = _spi.readRegister(0x30); result.value == 0x00) return true;
    else return false;
}


/// @brief 
/// @param address 
/// @param value 
/// @return
bool Transceiver::writeRegister(uint8_t address, uint8_t value)
{
   return _spi.writeRegister(address,value);
}

/// @brief  Reads a single register from the CC1101 transceiver.
/// @param address - The address of the register to read (0x00 to 0x3F).
/// @note: The first byte returned is the chip status byte, which indicates the current state of the CC1101.
/// @note: The second byte is the value of the specified register.  
/// @note: This function is useful for reading configuration registers or status information from the CC1101.
/// @return ReadResult - Contains the status byte and the value of the specified register.
ReadResult Transceiver::readRegister(uint8_t address)
{
    return _spi.readRegister(address);
}


/// @brief Writes multiple bytes to a specified address using burst write.
/// @param address - Target address (e.g., 0x7F for CC1101 burst write).    
/// @param data - Data Buffer to write
/// @param length - Number of bytes to write (for the CC1101 FIFO max. 64 bytes).
/// @return true if the write operation was successful
bool Transceiver::writeBurstRegister(uint8_t address, const uint8_t *data, size_t length)
{
    return _spi.writeBurstRegister(address,data,length);
}

/// @brief Useful for debugging to ensure the PATABLE configuration, set via writeBurstRegister, is correct.
///     How to Use and Interpret readPATable outside the class :  Read and verify PATABLE
///    uint8_t paTable[8];
///    if (transceiver.readPATable(paTable)) {
///        Serial.println("PATABLE Contents:");
///        for (int i = 0; i < 8; i++) {
///            Serial.print("PATABLE["); Serial.print(i); Serial.print("] = 0x");
///            Serial.println(paTable[i], HEX);
///        }
///        // Check for HIGH_POWER (index 7 = 0xC0)
///        if (paTable[7] == 0xC0) {
///            Serial.println("PATABLE[7] confirmed as 0xC0 (~10 dBm)");
///        } else {
///            Serial.println("Error: PATABLE[7] mismatch");
///        }
///    } else {
///        Serial.println("Error: Failed to read PATABLE");
///    }
/// @param paTable - Pointer to an 8-bytes array that store the PATABLE contents.
/// @return true if the read operation was successful
bool Transceiver::readBackPATABLE(uint8_t* paTable)
{
    #if LOG_VERBOSE
        LOG("Transceiver::readBackPATABLE() - Reading PATABLE contents");
        printDots(3, 500); // Print 3 dots with a 500 ms delay between each dot
        LOG("\n");
    #endif
    // Step 1: Sanity check for address
    if(CC1101::Address::PATABLE > bitFlags::AddressMask)
    {
        LOG_NEW_LINE("Error: Invalid PATABLE address");
        return false;
    }

    //  Step 2: Read the 8-bytes from the PATABLE using burst read
    // The CC1101 PATABLE is at address 0x3E, and we read 8 bytes starting from that address.
    // The readBurstRegister function will handle the SPI transaction and return the result.
    if( bool success = _spi.readBurstRegister(CC1101::Address::PATABLE, paTable, 8))
    {
      avr_algorithms::for_each_element( paTable, 8, [](uint8_t byte) {
          LOG_PAIR_HEX("PATABLE Byte", byte);
          return true; // success
      });
        return success;
    }else{
        LOG("Error: Failed to read PATABLE via burst read\n\n");
        return false;
    }
}


/// @brief 
/// @param value 
void Transceiver::writePATABLE()
{
    uint8_t index = _transceiver_config.getPATableIndex();
   configurePATable(index);
}


/// @brief CC1101’s PATABLE (at address 0x3E) is an 8-byte array storing up to 8 power levels (e.g., {0x03, 0x0D, 0x1C, 0x34, 0x51, 0x85, 0xC8, 0xC0} for 315 MHz). 
///     Each byte corresponds to a power output (e.g., 0xC0 at index 7 for ~10 dBm, HIGH_POWER).
///         FREND0 is an 8-bit register with the following structure (per the CC1101 datasheet, section 27.16):
///         - Bits 7–6: Reserved (typically 0).
///         - Bits 5–3: LODIV_BUF_CURRENT_TX (controls TX buffer current, often set by modulation settings like OOK/FSK).
///         - Bits 2–0: PA_POWER (selects PATABLE index, 0–7).
///
/// @param powerlevelIndex 
void Transceiver::configurePATable(uint8_t powerLevelIndex)
{
    // Step1: Validate index
    if (powerLevelIndex > 7) {
        LOG_NEW_LINE("Error: Invalid PATABLE index");
        return;
    }

    // Step2:  Write the entire 8-byte PATABLE using a burst write
    writeBurstRegister(CC1101::Address::PATABLE, paTable ,8);                 //

    // Step3:  Ensure the CC1101 uses the correct PATABLE entry Set FREND0.PA_POWER
    uint8_t frend0 = readRegister(CC1101::Address::FREND0).value;          // Reads the current value of the FREND0 register (address 0x22) and clears its PA_POWER bits (bits 2:0) while preserving other bits.
    frend0 &= ~0x07;                                                                            // Clear PA_POWER bits (bits 2:0)                                                                                  
    frend0 |= (powerLevelIndex & 0x07);                                                 // Sets the PA_POWER bits (2:0) in frend0 to the desired PATABLE index (powerLevelIndex), ensuring it’s within 0–7.   
    writeRegister(CC1101::Value::FREND0, frend0);                                            // Writes the updated frend0 value back to the FREND0 register.  
    
}


/// @brief Sends Strobe command thought a SPI transition
/// @param command Takes a enum for type-safety, so restrict input to defined strobes
/// @return True if the operation was successful
bool Transceiver::strobeCommand(CC1101::Strobes::Command command)
{
    bool success = false;

    avr_algorithms::repeat_withExitCondition(3, [&]() {
        // Step 1: Send strobe command
        _spi.applyTransaction([&]() {
            _spi.transferByte(static_cast<uint8_t>(command));
        });

        // Step 2: Check if the chip is responsive (PARTNUM == 0x00)
        auto partnum = readRegister(CC1101::Address::PARTNUM).value;
        if (partnum == 0x00) {
            success = true;
            LOG_DYNAMIC("Strobe Command " + String(avr_algorithms::toString(command)) + 
                        " (0x" + String(static_cast<uint8_t>(command), HEX) + 
                        ") was successfully sent.");
            return false; // Stop retries
        } else {
            String errorMsg = "CC1101 unresponsive after strobe command: " + String(avr_algorithms::toString(command)) + 
                            " (0x" + String(static_cast<uint8_t>(command), HEX) + 
                            "), PARTNUM: " + String(partnum, HEX);
            LOG_DYNAMIC(errorMsg);
            return true; // Retry
        }
    });

    if (!success) {
        LOG_NEW_LINE("Error: CC1101 unresponsive after strobe.");
    }
    return success;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------- Implemented method from ITransceiver ---------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// @brief Configures the Carrier frequency by  writing three Register FREQ2, FREQ1, FREQ0
///     This registers form a 24-bits word that determines the RF carrier frequency use for tranmission.
///     This word will typically be set to the centre of the lowest channel frequency that is to be used.
///     What this function does??
///     - Step1: Check if the frequency is within the supported range of the CC1101(300-928MHz).
///     - Step2: Convert that freq to a 24-bit word value base on the internal crystal oscillator of the CC1101(26MHz) 
///     - Step3: Program the Transceiver via SPI writing the three register FREQ2, FRQ1, FREQ0
/// @param frequencyHz - frequency in Hz 
void Transceiver::setFrequency(uint32_t frequencyHz)
{
    // Step1 : Frequency Validation
    if (frequencyHz < 300000000 || frequencyHz > 928000000) {
        LOG_NEW_LINE("Error: Frequency out of CC1101 range (300–928 MHz)");
        return;
    }

    // Step2: Define const oscillator freq
    constexpr uint32_t F_XOSC = 26000000;                                       //  Define the C1101 crystal oscillator as a reference for frequency synthesis ( typ. 26 MHz crystal) form datasheet, section 12
    
    // Step3: Converts the desired frequency (in Hz) to a 24-bit frequency control word (freq) used by the CC1101.
    uint32_t freq = (uint64_t)frequencyHz * (1ULL << 16) / F_XOSC;                                                                              // F_carrier = (Fx_OSC/2^16)*freq (datasheet, section 12)  => freq = (F_carrier *2^16)/FxOSC. [ (1ULL << 16) computes 2^16 = 65536, using ULL for 64-bits precision]
    
    writeRegister(CC1101::Address::FREQ2, (freq >> 16) & 0xFF);                                                                               // Extracts  (bits 23:16) to configure FREQ2
    writeRegister(CC1101::Address::FREQ1, (freq >> 8) & 0xFF);                                                                                 // Extracts  (bits 15:8) for FREQ1
    writeRegister(CC1101::Address::FREQ0, freq & 0xFF);                                                                                           // Extracts  (bits 7:0) for FREQ0
}

/// @brief Set the Power tranmission frequency
/// @param level - PaTable index 
void Transceiver::setPowerLevel(uint8_t level)
{
    if (level > 7) level = 7;
    configurePATable(level);
}

/// @brief Put the CC1101 module in SLEEP state
void Transceiver::sleep()
{
    strobeCommand(CC1101::Strobes::Command::SPWD);
}


/// @brief  Function  that decodes both the chip state and the FIFO
/// @param readResult 
/// @return 
StatusInfo Transceiver::decodeStatus(ReadResult readResult)
{
    StatusInfo statusInfo;
    
    statusInfo.fifoBytes  =  (readResult.status & bitFlags::FIFObytes);                  // Extract the number of bytes available in the RX FIFO or free bytes in the TX FIFO (3:0)
    statusInfo.chipState =(readResult.status >> 4) & bitFlags::ChipState;           // shift first: status >> 4 (moves CHIP_STATE into bits 3–0). Then mask with 0x0F to get just those 4 bits cleanly.

    return statusInfo;
}