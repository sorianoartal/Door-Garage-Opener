#include "SPI/SPIBus.h"
#include "utils/HelperFunc.h"


/// @brief SPIBus constructor
/// @param csnPin Pin for Chip Select (active low). Enables/disables the device for SPI communication.
/// @param clockSpeed Clock speed (Hz) for SPI communication (default: 1 MHz).
/// @param bitOrder Data bit order: MSBFIRST or LSBFIRST (default: MSBFIRST).
/// @param spiMode SPI mode (0–3) for clock phase and polarity (default: SPI_MODE0).
SPIBus::SPIBus(uint8_t csnPin, uint32_t clockSpeed, uint8_t bitOrder, uint8_t spiMode):
_csnPin(csnPin),
_settings(clockSpeed, bitOrder,spiMode)
{
}

/// @brief Initialize the SPI bus to ensures the SPI bus is ready and the CC1101 is deselected by default.
void SPIBus::begin()
{
    pinMode(_csnPin,OUTPUT);                                        // Configure CSn pin as output
    deselectDevice();                                                       // Deselect device as default state
    SPI.begin();                                                               // Initialize SPI 
}

/// @brief Set CSn pin LOW to select the device.
void SPIBus::selectDevice()
{   
    digitalWrite(_csnPin,LOW);                                           // Enable device to be ready for receiving data   
}

/// @brief Set CSn pin HIGH to deselect the device.
void SPIBus::deselectDevice()
{ 
    digitalWrite(_csnPin,HIGH);                                           // Disable Slave device from SPI bus  
}


/// @brief Transfer a single byte through the SPI bus and return the response. Suitable for Strobe commands
/// @param data - byte to send
/// @return Status byte
uint8_t SPIBus::transferByte(uint8_t data)
{
    // To store the received data after the SPI transfer
    uint8_t receivedData;
    
    // Apply SPI transaction
    applyTransaction([&](){ receivedData = SPI.transfer(data);});
    
    // Log the transfer operation
    #if LOG_VERBOSE
        LOG("SPIBus::transferByte - Single byte transfer");
        printDots(3, 500); // Print 3 dots with a 500 ms delay between each dot
        LOG_PAIR_HEX("Sent: ", data);
        LOG_PAIR_HEX("Received: ", receivedData);
         LOG("\n");
    #endif

    // Return the received data
    return receivedData;
}


/// @brief Writes multiple bytes to an addres( e.g CC1101 FIFO )
/// @param address - address Target address (e.g., 0x7F for CC1101 burst write).
/// @param data - Data Buffer to write
/// @param length - Number of bytes to write( for the CC1101 FIFO max. 64bytes)
bool SPIBus::writeBurstRegister(uint8_t address, const uint8_t *data, size_t length)
{   
    // Validate parameters
    if (!data || length == 0 || length>64)
    {
        LOG_NEW_LINE("writeBurstRegister Error : Invalid parameters");
        LOG_PAIR_HEX("Address: ", address);
        LOG_PAIR_HEX("Length: ", length);
        return false;
    }
    
    // Validate address
    if(address != 0x03 && address != 0x3F)
    {
        LOG_NEW_LINE("writeBurstRegister Error : Invalid address ");
        LOG_PAIR_HEX("Address: ", address);
        return false;
    }

    // Sends the address followed by data bytes in a loop.
    applyTransaction( [&]()
        {
            SPI.transfer(address | bitFlags::writeBurstRegister);                                                                   // bitFlags::writeBurstRegister (0x40) to set bit 6 for burst write mode
            for (size_t i = 0; i < length; i++) {
                SPI.transfer(data[i]);
            }
        });

      LOG("\n\n");

   return true; // Success
}

/// @brief Read Multiple bytes from an address
/// @param address - Target address from where to read (e.g., 0x7F for CC1101 burst read).
/// @param buffer - Buffer to store the read data
/// @param length - Number of byte to read( max. 64 for the  CC1101)10
bool SPIBus::readBurstRegister(uint8_t address, uint8_t *buffer, size_t length)
{
    uint8_t attempts = 0;                                                                                                      // Initialize attempts counter   

   // Step1: Validate parameters
   // The function checks if the address is valid, the buffer is not null, and the length is within the valid range (1 to 64 bytes).
   // If any of these conditions are not met, the function returns false.
   if (!validateParameters(address, buffer, length)) {
      LOG_NEW_LINE("readBurstRegister Error : Invalid parameters");
      LOG_PAIR_HEX("Address: ", address);
      LOG_PAIR_HEX("Length: ", length);
      return true; // Retry
   }

    // Step2: Retry mechanism for burst read
    // The function attempts to read the burst register up to 3 times, checking if the read was successful each time.
    avr_algorithms::repeat_withExitCondition(3, [&]()
    {      
        // Step3: Perform burst read operation
        if(performBurstRead(address, buffer, length))
        {
            LOG_NEW_LINE("SPIBus::readBurstRegister - Burst read successful");
            LOG_PAIR_HEX("Address: ", address);
            LOG_PAIR_HEX("Length: ", length);
            return false; // Exit condition: success
        }
        else
        {
            // Step4: Log error and retry
            String err = "SPIBus::readBurstRegister - Burst read failed at address 0x" + String(address, HEX) + ", attempt:  " + String(attempts);
            LOG_DYNAMIC(err);
            attempts++;
            return true; // Retry
        }
   });

   LOG("\n\n");

   // If we reach here, it means all retries failed
   return false;                                                                                                         // Return false to indicate failure after 3 attempts
}

/// @brief Writes a value to a CC1101 register over SPI
/// Sends the register address and value using SPI transfer, with chip-select 
/// toggling and proper transaction wrapping.
///
/// @param address - Register address : CC1101 register address (0x00 to 0x3F)
/// @param value - Byte to write into the register
/// @return true if reg was correctly write
bool SPIBus::writeRegister(uint8_t address, uint8_t value)
{
    ReadResult readResult;                                                                                                     // Initialize ReadResult to store status and value
    uint8_t attempts = 0;                                                                                                        // Initialize attempts counter

    // Step 1: Validate address
    // Validate address range and log error if invalid
    // The CC1101 register address must be within the range 0x00 to 0x3F (0 to 63 in decimal).
    // If the address is outside this range, the function logs an error message and returns false    
    // This validation ensures that the address is a valid CC1101 register address before attempting to write to it.
    if(address > bitFlags::AddressMask)
    {
      LOG("---------- SPIBus communication Error ---------");
      LOG_NEW_LINE("SPIBus::writeRegister Error: Invalid CC1101 register address");
      LOG_PAIR_HEX("Address: ", address);
      return false;                                                                                                                         // Validates address against bitFlags::AddressMask (0x3F) to ensure it’s a valid CC1101 register address (0x00–0x3F).
    }

    // Step 2: Retry mechanism for writing the register
    // The function attempts to write the register up to 3 times, checking if the write was successful each time.
    // If the write is successful, it returns true. If not, it logs an error message and retries up to 3 times.
    avr_algorithms::repeat_withExitCondition(3,[&]()
    {
        // Perform SPI write transaction
        applyTransaction([&]()
        {
            SPI.transfer(address & bitFlags::WriteSingle);                                                                      // bitFlags::WriteSingle (0x7F) to clear bit 7 for single-byte write mode (e.g., 0x02 & 0x7F = 0x02 for IOCFG0)
            SPI.transfer(value);
        });

        // Step 3: Read back to verify success  
        readResult = readRegister(address);                                                                                  // Read the register to verify the write operation
        if(readResult.isValid() && readResult.value == value)                                                           // Check if the read value matches the written value   
        {
            LOG_NEW_LINE("SPIBus::writeRegister - Write operation successful");
            LOG_PAIR_HEX("Address: ", address);
            LOG_PAIR_HEX("Value: ", value);
            return false; // Exit condition: success
        }
        else
        {
            // Step 4: Log if verification fails
            String err = "Register write mismatch at 0x" + String(address, HEX)
                + ". Expected: 0x" + String(value, HEX)
                + ", Read: 0x" + String(readResult.value, HEX);
            LOG_DYNAMIC(err);
            attempts++;       // Increment attempts counter
            return true;         // Retry
        }
    });

   // If we reach here, it means all retries failed
   LOG_NEW_LINE("SPIBus::writeRegister Error: Failed to write register after 3 attempts");  
   LOG("\n\n");
   return false;    
}


/// @brief Reads a single register from the CC1101 transceiver.
/// @param address - The address of the register to read (0x00 to 0x3F).
/// @note The first byte returned is the chip status byte, which indicates the current state of the CC1101.
/// @note The second byte is the value of the specified register.
/// @note This function is useful for reading configuration registers or status information from the CC1101.
/// @return ReadResult - Contains the status byte and the value of the specified register.
ReadResult SPIBus::readRegister(uint8_t address)
{
    ReadResult result(0xFF, 0xFF);

    // Step 1: Validate address (immediate exit if invalid)
    if (address > bitFlags::AddressMask) {
        LOG_NEW_LINE("SPIBus::readRegister Error: Invalid CC1101 register address");
        LOG_PAIR_HEX("Address: ", address);
        return result;
    }

    uint8_t attempts = 0;

    String msg = "SPIBus::readRegister - Attempting to read register 0x" + String(address,HEX) ;
    LOG_DYNAMIC(msg);
    printDots(3, 500); // Print 3 dots with a 500 ms delay between each dot


    // Read retry mechanism
    avr_algorithms::repeat_withExitCondition(3, [&]() {
        applyTransaction([&]() {      
            result.status = SPI.transfer(address | bitFlags::ReadSingle);
            result.value  = SPI.transfer(bitFlags::DummyByte);

            #if LOG_VERBOSE
                LOG_NEW_LINE("SPIBus::readRegister - Read operation");
                LOG_PAIR_HEX("Address: ", address);
                LOG_PAIR_HEX("Status: ", result.status);
                LOG_PAIR_HEX("Value: ", result.value);
            #endif   
            return false; // Transaction succeeded, exit retry loop
        });

        if (!result.isValid()) {
            LOG_NEW_LINE("SPIBus::readRegister Error: Invalid status byte (0xFF) or value (0xFF)");
            String errorMsg = "Attempt " + String(attempts + 1) + " failed.";
            delayMicroseconds(100);
            LOG_DYNAMIC(errorMsg);
            LOG("Retrying");
            printDots(3, 1000); // Print 3 dots with a 500 ms delay between each dot
            LOG("\n");

            attempts++; // Only increment on retry
            return true; // Retry
        }

        return false; // Success, exit loop
    });

    if (!result.isValid()) {
        LOG_NEW_LINE("SPIBus::readRegister Error: Failed to read register after 3 attempts");
        LOG("\n\n");
    }
    return result;
}

/**
 * @brief Validates the parameters for SPI operations.
 * This function checks if the address is within the valid range,
 * if the buffer is not null, and if the length is within the valid range (1 to 64 bytes).
 * It ensures that the parameters are suitable for SPI burst read/write operations.
 * 
 * 
 * @param address - The address of the register to read/write (0x00 to 0x3F). 
 * @param buffer - Pointer to the data buffer for read/write operations.
 * @param length - The number of bytes to read/write (1 to 64 bytes).
 * @return true - if all parameters are valid.
 * @note If any of the conditions are not met, the function returns false.
 * @return false - if any of the parameters are invalid.
 */
bool SPIBus::validateParameters(uint8_t address, const uint8_t *buffer, size_t length) const
{
     // Step1: Validate parameters
    // Check if the address is valid, the buffer is not null, and the length is within the valid range (1 to 64 bytes).
    // If any of these conditions are not met, the function returns false.
    if(length > 64 ) return false;                                                                                                    
    
    // Step2: Validate address
    // The CC1101 register address must be within the range 0x00 to 0x3F (0 to 63 in decimal).
    // If the address is outside this range, the function returns false.
    if ((address & bitFlags::AddressMask) > bitFlags::AddressMask) return false ;                          

    // Step3: Validate buffer
    // Check if the buffer is not null. If it is null, the function returns false
    if(!buffer) return false;                                 

    // if we reach here, all parameters are valid,return true.
    return true;                                                                                                
}

/**
 * @brief Performs a burst read operation over SPI.
 * This function reads multiple bytes from a specified address using SPI burst read.
 * It sends the address with the read burst register flag and reads the data bytes into the provided buffer.
 * 
 * @param address - The address of the register to read (0x00 to 0x3F).
 * @note The address is combined with the readBurstRegister flag (0xC0)
 * @param buffer - Pointer to the buffer where the read data will be stored.
 * @note The buffer must be allocated with sufficient size to hold the read data.
 * @param length - The number of bytes to read (1 to 64 bytes).
 * @note The length must be within the valid range (1 to 64 bytes).
 * @return true - if the burst read operation was successful and data was read into the buffer.
 * @note If the read operation fails (e.g., all bytes in the buffer are 0xFF), the function returns false.
 * @return false - if the burst read operation failed, indicating a likely SPI read failure.
 * @note The function uses the applyTransaction method to perform the SPI transfer, ensuring that the CSn pin is toggled correctly and the SPI settings are applied.
 */
bool SPIBus::performBurstRead(uint8_t address, uint8_t *buffer, size_t length)
{
    // Initialize flag to check if all bytes read are 0xFF
    bool allFFs = true; 

    // Step1: Read burst register
    // The function sends the address with the readBurstRegister flag (0xC0)
    // and reads the data bytes into the provided buffer.
    // The address is combined with the readBurstRegister flag to indicate a burst read operation.  
    // The SPI transfer is performed within an applyTransaction block to ensure proper CSn pin handling and SPI settings.
    // The applyTransaction method is used to perform the SPI transfer, ensuring that the CSn pin is toggled correctly and the SPI settings are applied.
    // The SPI.transfer function is used to send the address and read the data bytes into the buffer.
    // The function iterates over the buffer and checks if all bytes read are 0xFF.
    // If any byte is not 0xFF, the allFFs flag is set to false, indicating that the read operation was successful and data was read into the buffer.
    // If all bytes are 0xFF, it indicates a likely SPI read failure,
    applyTransaction([&]() {
        SPI.transfer(address | bitFlags::readBurstRegister);
        avr_algorithms::for_each(buffer, length, [&](uint8_t& data, uint8_t index) {
            data = SPI.transfer(bitFlags::DummyByte);
            if (data != 0xFF) allFFs = false;
        });
    });

    // Return true if not all bytes are 0xFF, indicating a successful read operation
    return (allFFs) ? false : true;  
}

/// @brief Converts internal status info into a human-readable diagnostic string.
/// Format: 
///              "CHIP STATE: RX (0x01), FIFO Bytes: 7"
///  - Uses chipStateToString() to decode symbolic meaning.
///  - Appends both hex and numeric values for trace/debug readability.
/// @return String with formatted chip state and FIFO content.
String StatusInfo::toString() const {

    // Decode chip state to symbolic name (e.g., "TX", "IDLE")
    String stateStr = chipStateToString(StatusInfo::chipState);                             

    // Build and return the complete formatted message
    return "CHIP STATE: " + stateStr +
           " (0x" + String(chipState, HEX) + "), FIFO Bytes: " + String(fifoBytes);
}