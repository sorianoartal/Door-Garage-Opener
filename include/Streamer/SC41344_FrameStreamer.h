#pragma once
#include "interfaces/IFrameStreamer.h"
#include "interfaces/IBitEncoder.h"
#include "Config/Constants.h"
#include "avr_algorithms.hpp"


// Forward declaration to avoid circular dependency
class Transceiver;

/**
 * @brief Implements a frame transmission strategy tailored for the SC41344 protocol.
 * 
 * This class breaks a message into a structured bitstream with a preamble, a payload,
 * and post-frame silence. It handles the precise timing and ordering required by
 * the SC41344 standard, typically used in key fobs or RF remote controls.
 * 
 * The bit encoding is delegated to an external IBitEncoder implementation which 
 * knows how to encode logical '1', '0', and 'open' bits using pulse-width modulation.
 * 
 * @tparam N Number of data bits in the frame (typically 8 for SC41344).
 * 
 * Usage:
 * --------
 *   SC41344_FrameStreamer<8> streamer;
 *   streamer.streamFrame(myCodeBits, myEncoder);
 * 
 * Dependencies:
 * - IBitEncoder: Encodes logical bits as RF pulses (e.g., HIGH-LOW patterns)
 * - Constants.h: Defines timing constants like FRAME_REPEATS
 */
template<size_t N>
class SC41344_FrameStreamer : public IFrameStreamer<N>
{
public:

    /// @brief 
    /// @param transceiver makes the FrameStreamer self-contained, capable of managing its own RF state if needed
    SC41344_FrameStreamer(Transceiver& transceiver): _transceiver(transceiver){}
    ~SC41344_FrameStreamer() override = default;

    /**
     * @brief Stream the whole code message bit by bit using the Encode object to generate the waveform
     * 
     * @param code_DataBits - Store the code logic bit as an Array of '0' & '1'
     * @param encoder - Know how is bit is encode for the different Protocols used
     */
    void streamFrame(const uint8_t (&code_DataBits)[N], IBitEncoder &encoder) override;   

   
    /**
     * @brief Static method to stream a frame using the SC41344 protocol.
     * 
     * This method is useful for testing purposes, allowing the frame to be streamed
     * without needing an instance of SC41344_FrameStreamer.
     * 
     * @param code_DataBits Array of logical bits (0 or 1) representing the frame content.
     * @param encoder       Object that encodes individual bits into physical waveforms.
     */
    static void streamFrameStatic(const uint8_t (&code_DataBits)[N], IBitEncoder &encoder) {
        auto sendBit = [&encoder](uint8_t bit) {
            if (bit == 1) encoder.sendOne();
            else encoder.sendZero();
        };

        auto sendFrame = [&]() {
            avr_algorithms::for_each(code_DataBits, sendBit);
            encoder.sendOpen();
        };

        encoder.sendPreamble();
        sendFrame();
        avr_algorithms::repeat(FRAME_REPEATS, [&]() {
            encoder.sendSilence();
            sendFrame();
        });

        encoder.setIdle();
    }

private:

    Transceiver& _transceiver;
};


// ---------------------------------------------------------------------------------

/**
 * @brief Streams a complete frame following the SC41344 protocol.
 * 
 * The frame is composed of:
 *   1. Preamble (sync pulse)
 *   2. First data word (encoded with encoder)
 *   3. One or more repeated words, each preceded by a silence
 * 
 * Each bit in the frame is encoded as either a '1', '0', or a special 'OPEN' 
 * tri-state using the injected IBitEncoder.
 *
 * @param code_DataBits Array of logical bits (0 or 1) representing the frame content.
 * @param encoder       Object that encodes individual bits into physical waveforms.
 */
template <size_t N>
inline void SC41344_FrameStreamer<N>::streamFrame(const uint8_t (&code_DataBits)[N], IBitEncoder &encoder)
{   
    #if LOG_VERBOSE 
        LOG_NEW_LINE("SC41344_FrameStreamer::streamFrame() - Streaming frame to CC1101...");
    #endif

    // ------------------------------------------------------------------
    // Lambda section: helpers to construct the Frame sequence
    // ------------------------------------------------------------------

    // Send each logical bit (0 or 1) via the encoder
    auto sendBit = [&encoder](uint8_t bit) {
        if (bit == 1)  encoder.sendOne();
        else             encoder.sendZero();
    };

   // Encodes a full word + open bit
    auto sendFrame = [&]()
    {
        avr_algorithms::for_each_element(code_DataBits,sendBit);
        encoder.sendOpen();
    };    

    // ----------------------------------------------
    //  Frame transmission construction Logic:
    //  - Preamble-> Frame -> silence -> Frame
    // ----------------------------------------------

    // Transmit full frame sequence: preamble + data + repeats
    encoder.sendPreamble();                         // Step1: Send the Preamble for sync with receiver
    sendFrame();                                          // Step 2: Send first frame

    // Repeat the word for the remaining n−1 times with silence in between  
    avr_algorithms::repeat(FRAME_REPEATS,[&]()                
    {
      encoder.sendSilence();                          // Gap between frames  
      sendFrame();                          
    });                                                         

    encoder.setIdle();                                   // After send the repeated frames, put the encoder output  in the IDLE state to be ready for the next transmission.                                            
}

