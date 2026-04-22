#pragma once

#include <stdint.h>         // include that defines fixed-width integer types — guaranteed to be the same size on every platform.

// ---------------------------------------------------------------------------------
//                                      Button door variables
// ---------------------------------------------------------------------------------
constexpr uint8_t REMOTE_BUTTON_ID = 0;
constexpr uint8_t BUTTON_HOME_DOOR_GARAGE_PIN = 3;

// ----------------------------------------------------------------------------------
// GDO0 pin that connected to the Transceiver module for OOK control 
// ----------------------------------------------------------------------------------
constexpr uint8_t GDO0_PIN = 8 ;               // D8 Arduino Nano pin 8
constexpr uint8_t GDO0_PORT_BIT = 0 ;       // PORTB bit 0 (D8, PB0 for CC1101 GDO0)



// ---------------------------------------------------------------------------
//                      Debounce technique parameters
// --------------------------------------------------------------------------
constexpr uint8_t THRESHOLD_DEBOUNCE = 60 ;                                                              
constexpr uint16_t SAMPLE_RATE_DEBOUNCE = 1000;  

// ---------------------------------------------------------------------------------
// Bit timing extract it from the SC41344 to mimic the waveform( from logic analyzer)
//      - How data is encoded:                                               
//            '0' :        It's encoded as two short consecutive pulses -> ..._| |_____| |__...
//            '1' :        It's encoded as two long consecutive pulses ->  ..._|     |__|     |__...
//            'OPEN':  As a long pulse follow by short pulse ->            ..._|     |__| |_____...     
// ---------------------------------------------------------------------------------
constexpr uint16_t SHORT_HIGH_US = 300;
constexpr uint16_t SHORT_LOW_US  = 300;
constexpr uint16_t LONG_HIGH_US  = 2200;
constexpr uint16_t LONG_LOW_US   = 2200;

// Preamble duration for sync with receiver
constexpr uint16_t PREAMBLE_LOW_DURATION_US = 10000; // us

// Each data period is 8 clock pulse long = 5ms
constexpr uint16_t DIGIT_PERIOD_US = 5000;              // Total duration per digit (includes padding after pulse)

// Between two repeated frames there's a silence of 3 DATA PERIODS
constexpr uint16_t FRAME_SILENCE_BETWEEN_WORDS = 3 * DIGIT_PERIOD_US;                           // 15 ms gap between complete frame retransmissions
constexpr uint8_t  FRAME_BIT_COUNT = 8;                                                         // Data bits -> SC41344 frame = 8 real bits + 1 OPEN (sent separately)
constexpr uint8_t  FRAME_TRANSMISSION_COUNT = 4;                                                // Total on-air frame emissions per button press: 1 initial frame + 3 retransmissions
constexpr float    CLOCK_FREQ_HZ = 1680.7;                                                      //  ForDebug




// ------------------------------------------------------------------------------------------------------------------------------
//                                                  CSn pin for SPI communication
// ------------------------------------------------------------------------------------------------------------------------------
constexpr uint8_t CSN_PIN = 10;             // API pin chip select

// -------------------------------------------------------------------------------------------------------------------------------
//                                          Parameter for the CC1101 Transceiver module
// -------------------------------------------------------------------------------------------------------------------------------
constexpr uint32_t FREQ_315MHZ_BAND = 315000000;
constexpr uint32_t FREQ_331MHZ_BAND = 331000000;
constexpr uint32_t FREQ_433MHZ_BAND = 433920000;
constexpr uint32_t FREQ_868MHZ_BAND = 868000000;    // 868 MHz


// CC1101 OOK uses PATABLE[0] for logic '0' and PATABLE[1] for logic '1'.
// For a true RF "off" state between pulses, logic '0' must be 0x00.
constexpr uint8_t OOK_LOGIC_ZERO_POWER_BYTE = 0x00;
constexpr uint8_t OOK_LOGIC_ONE_POWER_INDEX = 1;

// Low-band OOK PATABLE bytes for the logic '1' level, derived from the
// CC1101 datasheet's low-band PA table. These remain valid starting points
// for both 315 MHz and nearby carriers such as 331 MHz.
constexpr uint8_t LOW_BAND_OOK_LOGIC_ONE_POWER_LOW    = 0x34;          // -10 dBm
constexpr uint8_t LOW_BAND_OOK_LOGIC_ONE_POWER_MEDIUM = 0x85;          // +5 dBm
constexpr uint8_t LOW_BAND_OOK_LOGIC_ONE_POWER_HIGH   = 0xC2;          // +10 dBm


constexpr size_t NUM_REG_TO_CONFIG_CC1101 = 23;    // Keep this aligned with the low-band OOK register table; 24 would add one zero-initialized entry.
constexpr bool VERIFY = true;                                       // To flag  which register can be read so we can check for write error
constexpr bool SKIP_VERIFY = false;                              // To flag register that are only write or not have a reliable return read value   



