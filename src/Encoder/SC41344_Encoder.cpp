#include "Encoder/SC41344_Encoder.h"
#include "avr_algorithms.hpp" // For repeat function
#include "utils/HelperFunc.h" // For printDots function

SC41344_Encoder::SC41344_Encoder(DigitalPin& pinPort_GDO0) : _GDO0_pin(pinPort_GDO0)
{
}

/**
* @brief Initializes the encoder by setting the pin as OUTPUT.
*/
void SC41344_Encoder::begin()
{
    #ifdef LOG_VERBOSE
    printDots(5, 500); // Print 5 dots with a 500 ms delay between each dot
    LOG_NEW_LINE("SC41344_Encoder::begin() - Initializing Encoder");
    #endif

    _GDO0_pin.pinConfig(
        false, // As output
        false  // No internal pullup resistor enabled
    );

    // Keep the async data line low while the radio is idle so entering TX
    // does not create a stray carrier burst before the waveform starts.
    _GDO0_pin.writePin(LOW);
}

/**
 * @brief Send an encoded digital '1' as two long consecutive pulses.
 */
void SC41344_Encoder::sendOne()
{
    auto streamOneBitSeq = [&]()
    {
        _GDO0_pin.writePin(HIGH);
        delayMicroseconds(LONG_HIGH_US);
        _GDO0_pin.writePin(LOW);
        delayMicroseconds(SHORT_LOW_US);
    };

    avr_algorithms::repeat(2, streamOneBitSeq);
}

/**
 * @brief Send an encoded digital '0' as two short consecutive pulses.
 */
void SC41344_Encoder::sendZero()
{
    auto streamZeroBitSeq = [&]()
    {
        _GDO0_pin.writePin(HIGH);
        delayMicroseconds(SHORT_HIGH_US);
        _GDO0_pin.writePin(LOW);
        delayMicroseconds(LONG_LOW_US);
    };

    avr_algorithms::repeat(2, streamZeroBitSeq);
}

/**
 * @brief Send the tri-state OPEN symbol as a long pulse followed by a short pulse.
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
 * @brief Send an inter-frame silence by holding the line LOW.
 */
void SC41344_Encoder::sendSilence()
{
    // Keep the line low for the whole gap; the next frame will raise it on the
    // first symbol edge, which avoids an unwanted pre-burst.
    _GDO0_pin.writePin(LOW);
    delayMicroseconds(FRAME_SILENCE_BETWEEN_WORDS);
}

/**
 * @brief Send the long LOW preamble used to sync the receiver.
 */
void SC41344_Encoder::sendPreamble()
{
    _GDO0_pin.writePin(LOW);
    delayMicroseconds(PREAMBLE_LOW_DURATION_US);
}

/**
 * @brief Set the encoder to the non-transmitting idle state.
 */
void SC41344_Encoder::setIdle()
{
    // Hold the async data input low so a later TX entry starts from carrier-off.
    _GDO0_pin.writePin(LOW);
}
