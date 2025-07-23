#include "Encoder/SC41344_Encoder.h"
#include "avr_algorithms.hpp" // For repeat function
#include "utils/HelperFunc.h" // For printDots function

SC41344_Encoder::SC41344_Encoder(DigitalPin &pinPort_GDO0): _GDO0_pin(pinPort_GDO0)
{
}

 /**
* @brief Initializes the encoder by setting the pin as OUTPUT.
*/
void SC41344_Encoder::begin()
{
    #ifdef LOG_VERBOSE
    // Log the initialization process
    printDots(5, 500); // Print 5 dots with a 500 ms delay between each dot
    LOG_NEW_LINE("SC41344_Encoder::begin() - Initializing Encoder");
    #endif

    // Pin used for OOK modulation
    _GDO0_pin.pinConfig
    (
        false,                                  // As output
        false                                   // No internal Pullup resistor enable
    );

    // Set initial state High, so that we are ready t send a preamble
    _GDO0_pin.writePin(HIGH);
}

/**
 * @brief Send a encoded digital '1' :  As two long consecutive pulses ->  ..._|     |__|     |__... 
 * 
 */
void SC41344_Encoder::sendOne()
{
    // Define a lambda function to stream the sequence for '1'
    // This function will write HIGH for LONG_HIGH_US, then LOW for SHORT_LOW_US
   auto streamOneBitSeq = [&]()
   {
      _GDO0_pin.writePin(HIGH);
      delayMicroseconds(LONG_HIGH_US);
      _GDO0_pin.writePin(LOW);
      delayMicroseconds(SHORT_LOW_US);       
   };

    // Send encoded '1' 
    // Repeat the sequence twice to match the SC41344 encoding requirements
   avr_algorithms::repeat(2, streamOneBitSeq);

}

/**
 * @brief  Send an encoded '0' : As two short consecutive pulses -> ..._| |_____| |__...
 * 
 */
void SC41344_Encoder::sendZero()
{ 
    // Define a lambda function to stream the sequence for '0'
    auto streamZeroBitSeq = [&]()
    {
        _GDO0_pin.writePin(HIGH);
        delayMicroseconds(SHORT_HIGH_US);
        _GDO0_pin.writePin(LOW);
        delayMicroseconds(LONG_LOW_US);       
    };

    // Send encoded '0'
    // Repeat the sequence twice to match the SC41344 encoding requirements
    avr_algorithms::repeat(2,streamZeroBitSeq); 
}


/**
 * @brief Send the Tri-state 'OPEN' :  As a long pulse follow by short pulse ->       ..._|     |__| |_____...  
 * 
 */
void SC41344_Encoder::sendOpen()
{
    _GDO0_pin.writePin(HIGH);
    delayMicroseconds(LONG_HIGH_US);
    _GDO0_pin.writePin(LOW);
    delayMicroseconds(SHORT_LOW_US);
    _GDO0_pin.writePin(HIGH);
    delayMicroseconds(SHORT_HIGH_US);
    _GDO0_pin.writePin(LOW);
    delayMicroseconds(LONG_LOW_US);
}

/**
 * @brief Sends a silence period by setting the pin LOW for 15000 µs.
 * 
 */
void SC41344_Encoder::sendSilence()
{
    // Set the pin LOW for FRAME_SILENCE_BETWEEN_WORDS duration
    _GDO0_pin.writePin(LOW);
    delayMicroseconds(FRAME_SILENCE_BETWEEN_WORDS);
    _GDO0_pin.writePin(HIGH);
}

/**
 * @brief For sync the receiver's internal state a 10ms LOW so that is ready to read valid data
 * 
 */
void SC41344_Encoder::sendPreamble()
{
    // Set the pin LOW for PREAMBLE_LOW_DURATION_US duration
    _GDO0_pin.writePin(LOW);                                                
    delayMicroseconds(PREAMBLE_LOW_DURATION_US);        
}


/**
 * @brief Sets the encoder to IDLE state by setting the pin HIGH.
 * 
 */
void SC41344_Encoder::setIdle()
{
    
    // Set the pin HIGH to indicate IDLE state
    _GDO0_pin.writePin(HIGH);
}
