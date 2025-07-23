#pragma once

#include "interfaces/ITransceiver.h"
#include "Config/TransceiverConfig.h"
#include "SPI/SPIBus.h"
#include "Config/CC1101_Config/CC1101.h"
#include "Debugging/Logging.h"
#include "Config/CC1101_Config/CC1101_315MHZ_OOK_Config.h"
#include "utils/HelperConfigRegisters_CC1101.h"
#include "Streamer/SC41344_FrameStreamer.h"

#include "Policies/PROGMEMStoragePolicy.h"
#include "Policies/RAMStoragePolicy.h"
#include "Encoder/SC41344_Encoder.h"
#include "utils/HelperFunc.h"


#include<Arduino.h>

/**
 * @class Transceiver
 *
 * @brief Concrete implementation for the TI RF Transceiver Module  
 * 
 * Configure SPI communication: This class abstracts the low-level SPI-based communication between the Transceiver
 *  give you and agnostic-protocol interface for transmitting bitStreams(Command's).
 * 
 * It separates:
 *      - Hardware communication (SPI, Register config)
 *      - Protocol-specific for the data framing  encoding is delegated to the IBitEncoder.
 *      - Uses the encoder classes to delegate the Modulation/bitStream logic
 *   
 * Key Features:
 *   - Applies user-defined configuration (modulation, frequency, power level)
 *   - Sends command strobes (SIDLE, STX, etc.)
 *   - Streams encoded data using a pluggable frame encoder
 *
 * Dependencies:
 *   - SPIBus: Abstracted SPI communication layer
 *   - TransceiverConfig: User-defined config for freq/power/modulation
 *   - IBitEncoder: Defines bit-level encoding (ASK/OOK, Manchester, etc.)
 * 
 * @example
 *   Transceiver transceiver(spiBus, config);    
 */
class Transceiver : public ITransceiver
{
    public:

        /// @brief 
        /// @param spi - Hardware dependency of SPIBus to communicate with the CC1101
        /// @param config - Desired configuration parameters for the CC1101 (Freq, modulation scheme, power)
        Transceiver(  SPIBus& spi, const TransceiverConfig& config);

        // -----------------------------------------------
        //  Inherit method via ITransceiver interface
        // -----------------------------------------------
        bool begin() override;                                                                               // Initialize hardware: SPI,  Apply full config passed at construction.
        void setFrequency(uint32_t frequencyHz) override;                                      // Write the frequency registers to tune which is gonna be the carrier freq. used
        void setPowerLevel(uint8_t level) override;                                                  // Power level for the antenna  
        void sleep() override;                                                                                // To enter in sleep Mode to save Power       

        template<size_t N>
        bool transmitFrame(const uint8_t (&code_DataBits)[N], IBitEncoder& encoder);    // Coordinate data transmission with the Encoder frame Streamer
        bool readBackPATABLE(uint8_t* paTable);                                                          // Verify PATABLE buffer content   
        ReadResult readRegister(uint8_t address);                                                        // Read a specify register 
        static StatusInfo decodeStatus(ReadResult readResult);                                      // Decode and print the status byte for human-readable diagnostics. First byte returned after register read is the chip status byte    

    private:

        SPIBus& _spi;                                                                                            // Handles low level communication with the Module via SPI protocol   
        const TransceiverConfig& _transceiver_config;                                             // Store the configuration desired for the Transceiver module
       

        // --------------------------------------------------------------
        // Helper Private function for Specific register operation  
        // --------------------------------------------------------------
        void enableTransmitMode();                                                                       // Enables Transmit Mode Tx for the CC1101 to transmit data    
        bool writeRegister(uint8_t address , uint8_t value);                                        // Write a single register of the CC1101      
        bool writeBurstRegister(uint8_t address , const uint8_t* data, size_t leng);       // Write multiplebytes at once       
        void writePATABLE();                                                                                    // Load the PATABLE register that can hold up to eight user selected output power settings.
        void configurePATable(uint8_t powerlevelIndex);                                           // Configures the PATABLE for a specific power level transmission.   

        bool strobeCommand(CC1101::Strobes::Command command);                     // These commands are used to disable the crystal oscillator, enable receive mode, enable wake-on-radio etc
        void reset();                                                                                              // Apply full reset sequence.
        bool verifyChipId();                                                                                    // Check if the PARTNUM is 0x00 at it should be after reset
};


/**
 * @brief Stream a data frame using the SC41344 protocol via the provided encoder.
 * 
 * This function ensures the radio is in TX mode, sends the data using the
 * SC41344 frame format, and returns the chip to IDLE.
 * 
 * @tparam N Length of the frame array (number of bits).
 * @param code_DataBits Bit array representing the logical message.
 * @param encoder Encoder that formats the bitstream into signal pulses.
 * @return true if transmission and cleanup completed successfully.
 */
template <size_t N>
inline bool Transceiver::transmitFrame(const uint8_t (&code_DataBits)[N], IBitEncoder &encoder)
{
    // Alias to access strobes commands
    using Strobe = CC1101::Strobes::Command;

    // Ensure TX mode
    enableTransmitMode();

    /*
    // Check if the chip is in TX mode
    LOG_NEW_LINE("Checking if CC1101 is in TX mode...");
    if (readRegister(CC1101::Address::MARCSTATE).value != 0x13) {
        LOG_NEW_LINE("Error: Not in TX mode.");
        return false;
    }
    */
   
    // Stream Frame
    LOG_NEW_LINE("Streaming frame to CC1101...");
    // Create a FrameStreamer instance for the SC41344 protocol 
    // and stream the data bits using the provided encoder
    // This will handle the encoding and timing of the bits
    SC41344_FrameStreamer<N> streamer(*this);
    streamer.streamFrame(code_DataBits, encoder);

    // Return to IDLE
    if (!strobeCommand(Strobe::SIDLE)) { 
        LOG_NEW_LINE("Error: Failed to return to IDLE mode");
        return false;
    }
    return true;
}
