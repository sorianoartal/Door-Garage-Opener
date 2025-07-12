#include <Arduino.h>
#include <avr/wdt.h>

#include "avr_algorithms.hpp"
#include "App/RemoteCodes.h"
#include "Debounce/CircularDebounceBuffer.h"
#include "Encoder/SC41344_Encoder.h"
#include "Streamer/SC41344_FrameStreamer.h"
#include "Config/Constants.h"
#include "Config/DigitalPin.h"
#include <SPI.h>
#include "SPI/SPIBus.h"
#include "Transciever/CC1101_Transceiver.h"
#include "Encoder/SC41344_Encoder.h"
#include "Config/TransceiverConfig.h"
#include "Debugging/Logging.h"

// Global state flag
volatile bool buttonFlag = false;

// Timing for periodic status updates
unsigned long lastTimeSend = 0;
constexpr uint16_t SEND_INTERVAL = 1000;

// SPI instance for CC1101 communication
SPIBus spiBus(CSN_PIN);

// CC1101 Transceiver configuration  (315 MHz, OOK, high power)
TransceiverConfig config
(
FREQ_315MHZ_BAND,                                                         
ModulationScheme::OOK,                                                   
OutputPowerLevels::HIGH_POWER                                       
);

// SC41344 encoder for generating RF signal on GDO0 pin
DigitalPin  gdo0Pin('B',static_cast<uint8_t>(GDO0_PORT_BIT));          // PORTB bit 0 (D8, PB0)                            
SC41344_Encoder encoder(gdo0Pin);                                   


// Debounce buffer for button input
CircularDebounceBuffer debounce(
  REMOTE_BUTTON_ID,
  BUTTON_HOME_DOOR_GARAGE_PIN ,
  true,
  SAMPLE_RATE_DEBOUNCE 
);

// CC1101 Transceiver instance 
Transceiver transceiver
(
  spiBus,
  config
);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                  ISR's section
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Triggers on a "physical" FALLING edge of the butto
 *   - This ISR arms the debounce to in the next update call from the
 *    loop() start to sample the button Pin
 */
void rawISRbuttonPressed()
{
  debounce.startDebounce();
}



// --------------------------------------------------------------------
//                    Function prototypes
// --------------------------------------------------------------------

/**
 * @brief Prints paTable to check configuration *
 */
void printPATable(); 

/**
 * @brief Callback that handle the button Press after debounce
 *    This will be call only once by the debounce.update() whenever the 
 *  debouncer detect that the button is in a stable state which means
 *  the button is truly pressed. 
 */
void onButtonPressed();


////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//                                              Setup section 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {

   // Initialize Serial Communication
  Serial.begin(115200);
  delay(250);                                                                                                             // Stabilize serial                    
 
  // Disable watchdog to prevent reset during initialization
  wdt_disable();
  LOG_NEW_LINE("Watchdog disabled during initialization");  

  // Initialize CC1101 Transceiver
  LOG("System Booting");
  printDots(3,1000);                                                                                              // Print 3 dots with a 1000 ms delay between each dot

  if (! transceiver.begin())
  {
    LOG_NEW_LINE("Transceiver initialization failed");
  }
  else
  {
    LOG_NEW_LINE("Transceiver initialized successfully");
  }

  // Configure button pin with pull-up resistor
  pinMode(BUTTON_HOME_DOOR_GARAGE_PIN, INPUT_PULLUP);
  
  // Attach interrupt to the Button
  attachInterrupt(
    digitalPinToInterrupt(BUTTON_HOME_DOOR_GARAGE_PIN),
    rawISRbuttonPressed,
    FALLING
  );

  // Configure Debouncing parameters
  debounce.setThreshold(THRESHOLD_DEBOUNCE);                                                                                             
  debounce.addCallback(onButtonPressed);
  
  // Initialization encoder
  encoder.begin();
  LOG_NEW_LINE("Encoder initialized");

  // Verify PA_TABLE configuration
  printPATable();

  // Enable interrupts
  interrupts();

  // Enable watchDog(8s timeout)
  wdt_enable(WDTO_8S);
  LOG_NEW_LINE("Watchdog enabled (8s timeout)");
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                              Main loop section
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {

  // Prevent watchdog reset
  // If not periodically reset, it assumes the program is stuck (e.g., in an infinite loop) and resets the microcontroller.
  wdt_reset();

  // Runs the Debounce state machine:
  // - If the startDebounce() has been call, it will start sample the pin button each stablish delay.
  // - Each sample would be shifted in a buffer.
  // - It would count the true values inside the buffer and if >= threshold it will execute the callback
  // - then the buffer is clear a the debounce is disarm until the next rawISRbuttonPressed is triggered
  debounce.update();

  
  // Print CC1101 status every second
  unsigned long currentTime = millis();
  if (currentTime - lastTimeSend >= SEND_INTERVAL)
  {
     StatusInfo status = Transceiver::decodeStatus( 
      transceiver.readRegister(CC1101::Address::MARCSTATE)
    );
    LOG_DYNAMIC(status.toString());  
    LOG_NEW_LINE("");
    lastTimeSend = currentTime;        // Reset timer
  } 
  

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Reads and logs the CC1101 PATABLE (unused in waveform test).
 */
void printPATable()
{
  uint8_t patable[8];
  if (transceiver.readBackPATABLE(patable))
  {
      LOG_NEW_LINE("PATABLE content:");
      uint8_t index = 0;
      avr_algorithms::for_each_element(patable,[&](uint8_t data){
        String msg = "PATABLE[" + String(index++) + "] = 0x"  + String(data,HEX) ;
        LOG_DYNAMIC(msg);
      });
  }else
  {
    LOG_NEW_LINE("Failed to read PATABLE");
  }  
}


/**
 * @brief Callback for stable button state changes.
 * Generates waveform on D8 when pressed, disables watchdog during critical section.
 */
void onButtonPressed()
{      
  LOG_NEW_LINE("Button pressed → transmitting");

  // Disable interrupts for a timing-critical section
  noInterrupts();                                                                                                          
  
  // Disable watchDog to prevent reset during delays
  wdt_disable();                                                                                                           
  LOG_NEW_LINE(" WatchDog disable");  
  
  // Send Command
  if (transceiver.transmitFrame(REMOTE1_OPEN_DOOR_CODE, encoder))
  {
    LOG_NEW_LINE("Transmission successful");
  }
  else
  {
   LOG_NEW_LINE("Transmission failed");
  }



  // Reenable watchDog
  wdt_enable(WDTO_8S);                                                                                                
  LOG_NEW_LINE("WatchDog reenable");
  
  // Re-enable interrupts
  interrupts();                                                                                                                  
}
   
