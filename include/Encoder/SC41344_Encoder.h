#pragma once

#include <Arduino.h>
#include "Config/Constants.h"
#include "Config/DigitalPin.h"
#include "interfaces/IBitEncoder.h"
#include "Debugging/Logging.h"

/**
 * @class SC41344_Encoder
 * @brief Implement the IBitEncoder interface to generate SC41344-encoded waveform signals for garage door openers
 * 
 * This class controls a digital pin to produce the corresponding  waveform that represent a door garage opener command
 *  It uses the SC41344 encoding scheme, which involves specific timings for preamble, data bit's(logic '1' and '0'), open state, 
 *  silence ,and end transmission by return the encoder output pin to IDLE state.
 * 
 * @note @note The pin must be configured as OUTPUT via begin() before sending waveforms.
 * @note Timings are based on the SC41344 protocol 
 */
class SC41344_Encoder: public IBitEncoder
{
    public:

    /**
     * @brief Construct a new SC41344 encoder object
     * 
     * @param pinPort_GDO0 - DigitalPin object that holds a reference to the  port pin where the CC1101 GDO0 pin is connected
     */
    SC41344_Encoder(DigitalPin& pinPort_GDO0);

    /**
     * @brief Configs the member variable required 
     * 
     */
    void begin();

    // -------------------------------------
    // Inherit method vie IBitEncoder
    // -------------------------------------
    void sendOne() override;
    void sendZero() override;
    void sendOpen() override;
    void sendSilence() override;
    void sendPreamble()override;
    void setIdle() override;

    private:

    DigitalPin& _GDO0_pin;          // Reference to the DigitalPin object controlling the output pin (e.g., D8).

};