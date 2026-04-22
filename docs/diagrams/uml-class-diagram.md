# UML Class Diagram

```mermaid
classDiagram
    class MainApp {
        +setup()
        +loop()
        +onButtonPressed()
        +printPATABLE()
    }

    class CircularDebounceBuffer {
        +startDebounce()
        +update()
        +addCallback()
        +setThreshold()
    }

    class Transceiver {
        +begin()
        +transmitFrame()
        +readRegister()
        +readBackPATABLE()
        +setFrequency()
        +setPowerLevel()
        -reset()
        -configurePATable()
        -strobeCommand()
    }

    class SPIBus {
        +begin()
        +writeRegister()
        +readRegister()
        +writeBurstRegister()
        +readBurstRegister()
        +transferByte()
        -waitUntilReady()
        -applyTransaction()
    }

    class TransceiverConfig {
        +getFrequencyHz()
        +getModulationScheme()
        +getPowerLevel()
        +getOOKLogicOnePowerByte()
    }

    class SC41344_FrameStreamer~N~ {
        +streamFrame()
        +streamFrameOnceStatic()
        +streamFrameStatic()
    }

    class SC41344_Encoder {
        +begin()
        +sendOne()
        +sendZero()
        +sendOpen()
        +sendSilence()
        +sendPreamble()
        +setIdle()
    }

    class DigitalPin {
        +pinConfig()
        +writePin()
        +readPin()
    }

    class ConfigLowBandOOK {
        +setting_Regs
        +setting_Regs_pgm
    }

    MainApp --> CircularDebounceBuffer : uses
    MainApp --> Transceiver : uses
    MainApp --> SC41344_Encoder : uses
    MainApp --> TransceiverConfig : constructs
    Transceiver --> SPIBus : owns reference
    Transceiver --> TransceiverConfig : reads config
    Transceiver --> ConfigLowBandOOK : applies register table
    Transceiver --> SC41344_FrameStreamer~N~ : creates during transmit
    SC41344_FrameStreamer~N~ --> SC41344_Encoder : drives waveform via interface
    SC41344_Encoder --> DigitalPin : toggles GDO0
```

## Notes

- `MainApp` represents the object graph assembled in `src/main.cpp`.
- `ConfigLowBandOOK` represents the low-band startup register table in `include/Config/CC1101_Config/CC1101_LowBand_OOK_Config.h`.
- The frame streamer and encoder are intentionally separate: one defines frame structure, the other defines pulse timing.
