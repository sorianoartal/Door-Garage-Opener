#pragma once

#include "interfaces/IBitEncoder.h"

/**
 * @brief Define the responsibility for Streamers classes to implement the streamFrame() Method 
 * 
 * @tparam N - number of bit to stream(code length), so that we can reuse the same interface for 4-bit, 8-bit, 12-bit frames, etc.
 */
template<size_t N>
class IFrameStreamer
{
  public:

    // Destruction to ensure proper cleanup
    virtual  ~IFrameStreamer() = default;

    /**
     * @brief Stream the whole code message bit by bit using the Encode object to generate the waveform
     * 
     * @param code_DataBits - Store the code logic bit as an Array of '0' & '1'
     * @param encoder - Know how is bit is encode for the different Protocols used
     */
    virtual void streamFrame(const uint8_t (&code_DataBits)[N], IBitEncoder& encoder ) = 0;

};