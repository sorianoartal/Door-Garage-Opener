#pragma once

#include <Arduino.h>
#include <SPI.h>                                                                // To handle low level SPI communocation. https://docs.arduino.cc/learn/communication/spi/
#include "Config/CC1101_Config/CC1101_SPI_Config.h"
#include "Debugging/Logging.h"
#include "Debugging/ChipStateUtil.h"
#include "avr_algorithms.hpp"

namespace bitFlags  = SPI_MASK::Masks;

/// @brief ReadResult structure to hold the status and value of a register read operation
/// @note This structure is used to encapsulate the result of a register read operation from the
/// CC1101 transceiver. It contains a status byte and a value byte, allowing for easy checking of the read operation's success.
/// @note The status byte indicates the state of the CC1101 chip, while the value byte contains the actual value of the specified register.
/// @note The status byte is expected to be 0xFF if the read operation was not successful, and the value byte is also expected to be 0xFF in that case.
/// @note The 'isValid()' function checks if both the status and value are not 0xFF, indicating a successful read operation.
/// @struct ReadResult
struct ReadResult {
    public:
    // Constructor to initialize the status and value
    ReadResult(uint8_t status = 0xFF, uint8_t value = 0xFF)
        : status(status), value(value) {}   

    // Helper function to check if the read operation was successful    
    bool isValid() const {
        return status != 0xFF && value != 0xFF; // Check if both status and value are not 0xFF
    }
    
    uint8_t status;             // Status byte returned
    uint8_t value;              // Value of the specified register  
};

/// @brief  StatusInfo structure to hold the chip state and FIFO bytes
/// @note  The chipState is the upper nibble (bits 7–4)
/// @note  The fifoBytes is the lower nibble (bits 3–0)
/// @note  This structure is used to decode the status byte returned by the CC1101 transceiver
/// @note  It provides a convenient way to access the chip state and FIFO byte count in
struct StatusInfo {
    uint8_t chipState;                                                              // bits 7–4
    uint8_t fifoBytes;                                                              // bits 3–0
    String toString() const;                                                    // Helper to pretty-print it
};


/**
 * @brief SPIBus class
 * * This class provides an abstraction for SPI communication with devices like the CC1101 transceiver.
 * * It handles:
 * *   - Initialization and configuration of the SPI bus
 * *   - Device selection and deselection using a Chip Select (CSn) pin
 * *   - Single-byte and burst read/write operations
 * * @note The SPIBus class is designed
 * * to be used with devices that support SPI communication, such as the CC1101 transceiver.
 * 
 * 
 * @example Use example:
 *   #include <SPI.h>
 *   #include "SPI/SPIBus.h"
 *  
 *  SPIBus spiBus(CSN_PIN); // Create an instance of SPIBus with the CSn pin
 *  spiBus.begin(); // Initialize the SPI bus
 *  spiBus.selectDevice(); // Select the device by setting CSn LOW
 *  uint8_t response = spiBus.transferByte(0x01); // Transfer a single byte
 *  spiBus.deselectDevice(); // Deselect the device by setting CSn HIGH
 *  
 *  // Burst write example
 *  uint8_t data[] = {0x01, 0x02, 0x03};
 * uint_8_t length = sizeof(data)/sizeof(data[0]);
 *  spiBus.writeBurstRegister(0x02, data, length); // Write multiple bytes to address 0x02
 *  
 *  // Burst read example
 *  uint8_t buffer[3];
 *  spiBus.readBurstRegister(0x02, buffer, sizeof(buffer)/sizeof(buffer[0]));   // Read multiple bytes from address 0x02
 *  
 *  // Read a single register
 *  ReadResult result = spiBus.readRegister(0x02); // Read a single register at address 0x02
 *  
 *  // Check the status and value
 *  Serial.print("Status: 0x"); Serial.println(result.status, HEX); // Print the status byte
 *  Serial.print("Value: 0x"); Serial.println(result.value, HEX); // Print the value of the register
 *  spiBus.end(); // Disable the SPI bus
 * 
 *  @note This class is designed to be used with the Arduino SPI library and provides a simple interface for SPI communication.
 *  @note The SPIBus class is not responsible for the specific protocol used by the device (e.g., CC1101 transceiver). It only provides the low-level SPI communication functions.
 */
class SPIBus 
{
    public:

        /// @brief SPIBus constructor
        /// @param csnPin -  Pin on each device that the Controller can use to enable and disable specific devices. When a device's Chip Select pin is low, it communicates with the Controller. When it's high, it ignores the Controller. This allows you to have multiple SPI devices sharing the same CIPO, COPI, and SCK lines.
        /// @param clockSpeed - Clock speed to synchronize SPI communication
        /// @param bitOrder - If the data shifted in Most Significant Bit (MSB) or Least Significant Bit (LSB) first
        /// @param spiMode -  there are four modes of transmission. These modes control whether data is shifted in and out on the rising or falling edge of the data clock signal (called the clock phase).
        SPIBus(uint8_t csnPin, uint32_t clockSpeed = 500000 , uint8_t bitOrder = MSBFIRST, uint8_t spiMode = SPI_MODE0 );

        void begin();                                                                                               // Initialization of SPI config.
        void end();                                                                                                  // Disables the SPI bus (leaving pin modes unchanged).
        void selectDevice();                                                                                      // Toggle CSn pin LOW to start a transaction.   
        void deselectDevice();                                                                                  // Toggle CSn pin HGH to end a transaction.   
        uint8_t transferByte( uint8_t data);                                                                 // Single-byte transmission
        bool writeBurstRegister(uint8_t address,const uint8_t* data , size_t length);      // Burst write operation for transfer multiple byte at once
        bool readBurstRegister(uint8_t address, uint8_t* buffer, size_t length);            // Burst read
        bool writeRegister(uint8_t address , uint8_t value);                                        // Write a single register
        ReadResult readRegister(uint8_t address);                                                   // Read a single register   

        template<typename Func>
        inline void applyTransaction( Func&& operation);

    private:

        bool validateParameters(uint8_t address, const uint8_t* buffer, size_t length) const;   // Validate parameters for burst read/write operations
        bool performBurstRead(uint8_t address, uint8_t* buffer, size_t length);                     // Perform burst read operation

        uint8_t _csnPin;                    // Pin on each device that the Controller can use to enable and disable specific devices. When a device's Chip Select pin is low, it communicates with the Controller. When it's high, it ignores the Controller. Chip select Pin(active low).
        SPISettings _settings;           // SPI configuration settings object for clock, bit order , and mode.  
};


/// @brief Helper function that wraps transfer SPI transition to avoid duplicate code
/// @tparam Func - Type of the operation that we need for 
/// @param operation 
template <typename Func>
inline void SPIBus::applyTransaction(Func &&operation)
{
    SPI.beginTransaction(_settings);             // To begin using the SPI port. The SPI port will be configured our settings. The simplest and most efficient way to use SPISettings is directly inside SPI.beginTransaction()
    selectDevice();                                     // Write the CSn LOW to prepare the device for the transition
    operation();                                        //  This will be the type of  transaction  function to apply
    deselectDevice();                                 // write the CSn HIGH to disable the device 
    SPI.endTransaction();                           // End using SPI port after finish   
};
