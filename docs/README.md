# DoorGarageFob Technical Overview

## Purpose

This project emulates the RF waveform of a garage-door remote by:

1. Debouncing a physical button on the Arduino Nano.
2. Bringing up a CC1101 transceiver with a low-band asynchronous OOK profile and a runtime-selectable carrier.
3. Driving the CC1101 `GDO0` pin with a software-generated SC41344-compatible waveform.
4. Repeating the frame several times to improve the chance that the receiver captures it.

## Hardware Mapping

| Function | Arduino / Module Pin | Source |
|---|---|---|
| Button input | `D3` | `BUTTON_HOME_DOOR_GARAGE_PIN` in `include/Config/Constants.h` |
| CC1101 `CSn` | `D10` | `CSN_PIN` in `include/Config/Constants.h` |
| CC1101 async TX data | `D8 / PB0` wired to `GDO0` | `GDO0_PIN` and `GDO0_PORT_BIT` |
| SPI bus | Arduino hardware SPI | `SPIBus` |

## Main Software Blocks

### `main.cpp`

`src/main.cpp` owns the application wiring:

- Constructs `SPIBus`, `TransceiverConfig`, `Transceiver`, `DigitalPin`, `SC41344_Encoder`, and `CircularDebounceBuffer`.
- Runs the transceiver bring-up during `setup()`.
- Only enables the encoder after radio configuration succeeds.
- Uses the button ISR only to arm the debounce logic.
- Lets `loop()` drive the debounce state machine and periodic radio status reads.

### `CircularDebounceBuffer`

`CircularDebounceBuffer` samples the button over time instead of trusting the first edge.

- A falling-edge ISR calls `startDebounce()`.
- `update()` samples the pin at the configured interval.
- The buffer confirms a stable press only when the threshold percentage is reached.
- Registered callbacks fire once per confirmed press.

### `Transceiver`

`Transceiver` is the high-level CC1101 controller.

- Initializes SPI.
- Performs the CC1101 manual reset sequence.
- Applies the register table from `Config_LowBand_OOK`.
- Programs PATABLE so OOK logic `0` is carrier-off and logic `1` uses the selected low-band OOK power byte.
- Switches the radio into TX and IDLE with strobe commands.
- Provides readback utilities such as `readRegister()` and `readBackPATABLE()`.

### `SPIBus`

`SPIBus` is the low-level transport wrapper around Arduino SPI.

- Starts and ends SPI transactions.
- Asserts and deasserts `CSn`.
- Waits for the CC1101 ready signal on `MISO` after `CSn` goes low.
- Handles single-register reads and writes.
- Handles burst access for PATABLE and FIFO-style operations.
- Produces the detailed debug logs used during bring-up.

### `SC41344_Encoder`

`SC41344_Encoder` is the waveform generator for the garage protocol.

- Drives the MCU pin connected to CC1101 `GDO0`.
- Encodes logical `1`, `0`, `OPEN`, preamble, silence, and idle states.
- Uses the timing constants in `include/Config/Constants.h`.

### `SC41344_FrameStreamer`

`SC41344_FrameStreamer` builds the full over-the-air message shape.

- Sends the preamble.
- Streams the data bits.
- Appends the `OPEN` symbol.
- Repeats the frame with a silence gap between repetitions.

## Startup Flow

The runtime initialization path is:

1. `setup()` starts Serial and disables the watchdog.
2. `Transceiver::begin()` initializes `SPIBus`.
3. `Transceiver::reset()` performs the CC1101 manual power-on reset sequence.
4. `applyRegisterConfig_CC1101()` writes the low-band OOK register table.
5. `configurePATable()` writes the 8-byte PATABLE, sets entry `0` to carrier-off, stores the selected logic-`1` level in entry `1`, and points `FREND0` at that high entry.
6. `Transceiver::begin()` verifies `PARTNUM` and `VERSION`.
7. Only after the radio is configured does `encoder.begin()` claim the MCU output pin that drives `GDO0`.
8. The watchdog is re-enabled and the system enters the normal loop.

## Transmission Flow

When the button is pressed:

1. The ISR only requests a debounce session.
2. `CircularDebounceBuffer::update()` confirms a stable press.
3. The registered callback `onButtonPressed()` runs.
4. Interrupts and the watchdog are temporarily disabled for timing stability.
5. `Transceiver::transmitFrame()` cycles the radio through repeated `IDLE -> TX -> IDLE` bursts.
6. `SC41344_FrameStreamer` asks `SC41344_Encoder` to toggle `GDO0` for each burst.
7. The CC1101 uses that async data stream to modulate the selected carrier, currently `331 MHz`.
8. The transceiver returns to IDLE after every repeated frame.
9. Interrupts and watchdog are restored.

## CC1101 Configuration Strategy

The project uses asynchronous OOK, so the CC1101 is not packetizing bytes for us. Instead:

- The register table configures the radio with a low-band OOK base profile.
- `PKTCTRL0` disables normal packet handling for async serial mode.
- `GDO0` is used as the software-driven async data input.
- The project directly generates the waveform with the encoder instead of writing bytes into TX FIFO.

This keeps the transmit path simple and gives precise control over the remote protocol timing.

## Timing Values Used by the Encoder

The important waveform constants are:

- Short high: `300 us`
- Short low: `300 us`
- Long high: `2200 us`
- Long low: `2200 us`
- Preamble low: `10000 us`
- Inter-frame silence: `15000 us`
- Total transmitted frames per button press: `4` (`1` initial + `3` repeats)

These values are stored in `include/Config/Constants.h`.

## Files To Read First

If you are onboarding into the project, start here:

1. `src/main.cpp`
2. `src/Transciever/CC1101_Transceiver.cpp`
3. `src/SPI/SPIBus.cpp`
4. `src/Encoder/SC41344_Encoder.cpp`
5. `include/Config/CC1101_Config/CC1101_LowBand_OOK_Config.h`

## Related Docs

- [CC1101 PDF guide](CC1101_Guide.pdf)
- [CC1101 PDF guide source](CC1101_Guide.md)
- [CC1101 stabilization and debugging history](cc1101-stabilization.md)
- [UML class diagram](diagrams/uml-class-diagram.md)
- [Runtime sequence diagrams](diagrams/runtime-sequence.md)
