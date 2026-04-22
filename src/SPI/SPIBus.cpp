#include "SPI/SPIBus.h"
#include "Config/CC1101_Config/CC1101.h"
#include "utils/HelperFunc.h"

namespace {
String registerLabel(uint8_t address)
{
    return String(CC1101::registerName(address)) + String(F(" (")) + formatHex8(address) + ')';
}

String attemptLabel(uint8_t attempt, uint8_t total = 3)
{
    return String(attempt) + "/" + String(total);
}
} // namespace


/// @brief SPIBus constructor
/// @param csnPin Pin for Chip Select (active low). Enables/disables the device for SPI communication.
/// @param clockSpeed Clock speed (Hz) for SPI communication (default: 1 MHz).
/// @param bitOrder Data bit order: MSBFIRST or LSBFIRST (default: MSBFIRST).
/// @param spiMode SPI mode (0-3) for clock phase and polarity (default: SPI_MODE0).
SPIBus::SPIBus(uint8_t csnPin, uint32_t clockSpeed, uint8_t bitOrder, uint8_t spiMode):
_csnPin(csnPin),
_settings(clockSpeed, bitOrder,spiMode)
{
}

/// @brief Initialize the SPI bus to ensures the SPI bus is ready and the CC1101 is deselected by default.
void SPIBus::begin()
{
    pinMode(_csnPin,OUTPUT);
    deselectDevice();
    SPI.begin();
}

void SPIBus::beginBus()
{
    SPI.beginTransaction(_settings);
}

void SPIBus::endBus()
{
    SPI.endTransaction();
}

/// @brief Set CSn pin LOW to select the device.
void SPIBus::selectDevice()
{
    digitalWrite(_csnPin,LOW);
}

/// @brief Set CSn pin HIGH to deselect the device.
void SPIBus::deselectDevice()
{
    digitalWrite(_csnPin,HIGH);
}

bool SPIBus::waitUntilReady(uint16_t timeoutUs) const
{
    unsigned long start = micros();
    while (digitalRead(MISO) == HIGH) {
        if (static_cast<unsigned long>(micros() - start) > timeoutUs) {
            return false;
        }
    }
    return true;
}


/// @brief Transfer a single byte through the SPI bus and return the response. Suitable for Strobe commands
/// @param data - byte to send
/// @return Status byte
uint8_t SPIBus::transferByte(uint8_t data)
{
    uint8_t receivedData = 0xFF;

    if (!applyTransaction([&](){ receivedData = SPI.transfer(data); })) {
        LOG_NEW_LINE("[SPI][BYTE][ERROR] Timed out waiting for CC1101 ready");
        return 0xFF;
    }

    #if LOG_VERBOSE
    LOG_DYNAMIC(String(F("[SPI][BYTE] TX ")) + formatHex8(data) +
                String(F(", RX ")) + formatHex8(receivedData));
    #endif

    return receivedData;
}

uint8_t SPIBus::transferRaw(uint8_t data)
{
    return SPI.transfer(data);
}


/// @brief Writes multiple bytes to an addres( e.g CC1101 FIFO )
/// @param address - address Target address (e.g., 0x7F for CC1101 burst write).
/// @param data - Data Buffer to write
/// @param length - Number of bytes to write( for the CC1101 FIFO max. 64bytes)
bool SPIBus::writeBurstRegister(uint8_t address, const uint8_t *data, size_t length)
{
    if (!data || length == 0 || length > 64)
    {
        LOG_DYNAMIC(String(F("[SPI][BURST-WRITE][ERROR] Invalid parameters for ")) +
                    registerLabel(address) + String(F(", length=")) + String(length));
        return false;
    }

    if (address > bitFlags::AddressMask)
    {
        LOG_DYNAMIC(String(F("[SPI][BURST-WRITE][ERROR] Invalid address ")) + formatHex8(address));
        return false;
    }

    #if LOG_VERBOSE
    LOG_DYNAMIC(String(F("[SPI][BURST-WRITE] ")) + registerLabel(address) +
                String(F(", length=")) + String(length));
    #endif

    if (!applyTransaction([&]()
    {
        SPI.transfer(address | bitFlags::writeBurstRegister);
        for (size_t i = 0; i < length; i++) {
            SPI.transfer(data[i]);
        }
    })) {
        LOG_DYNAMIC(String(F("[SPI][BURST-WRITE][ERROR] Timeout while writing ")) + registerLabel(address));
        return false;
    }

    LOG("\n\n");
    return true;
}

/// @brief Read Multiple bytes from an address
/// @param address - Target address from where to read (e.g., 0x7F for CC1101 burst read).
/// @param buffer - Buffer to store the read data
/// @param length - Number of byte to read( max. 64 for the  CC1101)10
bool SPIBus::readBurstRegister(uint8_t address, uint8_t *buffer, size_t length)
{
    uint8_t attempts = 0;
    bool success = false;

    if (!validateParameters(address, buffer, length)) {
        LOG_DYNAMIC(String(F("[SPI][BURST-READ][ERROR] Invalid parameters for ")) +
                    registerLabel(address) + String(F(", length=")) + String(length));
        return false;
    }

    avr_algorithms::repeat_withExitCondition(3, [&]()
    {
        if (performBurstRead(address, buffer, length))
        {
            #if LOG_VERBOSE
            LOG_DYNAMIC(String(F("[SPI][BURST-READ][OK] ")) + registerLabel(address) +
                        String(F(", length=")) + String(length));
            #endif
            success = true;
            return false;
        }

        LOG_DYNAMIC(String(F("[SPI][BURST-READ][RETRY ")) + attemptLabel(attempts + 1) +
                    String(F("] ")) + registerLabel(address) +
                    String(F(", length=")) + String(length));
        attempts++;
        return true;
    });

    LOG("\n\n");
    return success;
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
    uint8_t attempts = 0;
    String label = registerLabel(address);

    if (address > 0x2F)
    {
        LOG_DYNAMIC(String(F("[SPI][WRITE][ERROR] Invalid configuration register address ")) +
                    formatHex8(address));
        return false;
    }

    #if LOG_VERBOSE
    LOG_DYNAMIC(String(F("[SPI][WRITE] ")) + label + String(F(" <= ")) + formatHex8(value));
    #endif

    avr_algorithms::repeat_withExitCondition(3, [&]()
    {
        if (applyTransaction([&]()
        {
            SPI.transfer(address & bitFlags::WriteSingle);
            SPI.transfer(value);
        })) {
            #if LOG_VERBOSE
            LOG_DYNAMIC(String(F("[SPI][WRITE][OK] ")) + label +
                        String(F(" <= ")) + formatHex8(value));
            #endif
            return false;
        }

        LOG_DYNAMIC(String(F("[SPI][WRITE][RETRY ")) + attemptLabel(attempts + 1) +
                    String(F("] ")) + label +
                    String(F(": timeout waiting for CC1101 ready")));
        attempts++;
        return true;
    });

    if (attempts < 3) {
        return true;
    }

    LOG_DYNAMIC(String(F("[SPI][WRITE][ERROR] Failed to program ")) + label + F(" after 3 attempts"));
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
    String label = registerLabel(address);

    if (address > bitFlags::AddressMask) {
        LOG_DYNAMIC(String(F("[SPI][READ][ERROR] Invalid CC1101 register address ")) +
                    formatHex8(address));
        return result;
    }

    uint8_t attempts = 0;

    #if LOG_VERBOSE
    LOG_DYNAMIC(String(F("[SPI][READ] Request ")) + label);
    #endif

    avr_algorithms::repeat_withExitCondition(3, [&]() {
        uint8_t header = (address >= 0x30) ? static_cast<uint8_t>(address | bitFlags::readBurstRegister)
                                           : static_cast<uint8_t>(address | bitFlags::ReadSingle);

        if (!applyTransaction([&]() {
            result.status = SPI.transfer(header);
            result.value  = SPI.transfer(bitFlags::DummyByte);
        })) {
            LOG_DYNAMIC(String(F("[SPI][READ][RETRY ")) + attemptLabel(attempts + 1) +
                        String(F("] ")) + label +
                        String(F(" timed out waiting for CC1101 ready")));
            delayMicroseconds(100);
            attempts++;
            return true;
        }

        #if LOG_VERBOSE
        LOG_DYNAMIC(String(F("[SPI][READ][OK] ")) + label +
                    String(F(" -> value ")) + formatHex8(result.value) +
                    String(F(", status ")) + formatHex8(result.status));
        #endif

        if (!result.isValid()) {
            LOG_DYNAMIC(String(F("[SPI][READ][RETRY ")) + attemptLabel(attempts + 1) +
                        String(F("] ")) + label +
                        String(F(" returned invalid response (status ")) + formatHex8(result.status) +
                        String(F(", value ")) + formatHex8(result.value) + ')');
            delayMicroseconds(100);
            attempts++;
            return true;
        }

        return false;
    });

    if (!result.isValid()) {
        LOG_DYNAMIC(String(F("[SPI][READ][ERROR] Failed to read ")) + label + F(" after 3 attempts"));
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
    if(length == 0 || length > 64 ) return false;                                                                                                    
    
    // Step2: Validate address
    // The CC1101 register address must be within the range 0x00 to 0x3F (0 to 63 in decimal).
    // If the address is outside this range, the function returns false.
    if (address > bitFlags::AddressMask) return false ;                          

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
    if (!applyTransaction([&]() {
        SPI.transfer(address | bitFlags::readBurstRegister);
        avr_algorithms::for_each(buffer, length, [&](uint8_t& data, uint8_t index) {
            data = SPI.transfer(bitFlags::DummyByte);
            if (data != 0xFF) allFFs = false;
        });
    })) {
        return false;
    }

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
