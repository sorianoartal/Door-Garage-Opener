# Runtime Sequence Diagrams

## Startup And Radio Bring-Up

```mermaid
sequenceDiagram
    participant Setup as setup()
    participant Trx as Transceiver
    participant SPI as SPIBus
    participant Chip as CC1101
    participant Helper as applyRegisterConfig_CC1101
    participant Enc as SC41344_Encoder

    Setup->>Trx: begin()
    Trx->>SPI: begin()
    Trx->>Trx: reset()
    Trx->>SPI: select/deselect CSn sequence
    SPI->>Chip: SRES
    Chip-->>SPI: MISO ready
    Trx->>SPI: readRegister(PARTNUM)
    SPI->>Chip: read PARTNUM
    Chip-->>SPI: 0x00
    Trx->>Helper: apply register table
    loop each config register
        Helper->>SPI: writeRegister(addr, value)
        opt VERIFY flag is true
            Helper->>SPI: readRegister(addr)
            SPI->>Chip: read back value
            Chip-->>SPI: register contents
        end
    end
    Trx->>SPI: writeBurstRegister(PATABLE)
    Trx->>SPI: readRegister(FREND0)
    Trx->>SPI: writeRegister(FREND0 with PA index)
    Trx->>SPI: readRegister(PARTNUM)
    Trx->>SPI: readRegister(VERSION)
    Setup->>Enc: begin() only after radio init succeeds
```

## Confirmed Button Press To RF Transmission

```mermaid
sequenceDiagram
    participant ISR as rawISRbuttonPressed()
    participant Deb as CircularDebounceBuffer
    participant Loop as loop()
    participant Callback as onButtonPressed()
    participant Trx as Transceiver
    participant Streamer as SC41344_FrameStreamer
    participant Enc as SC41344_Encoder
    participant Chip as CC1101

    ISR->>Deb: startDebounce()
    Loop->>Deb: update()
    Deb-->>Callback: confirmed press callback
    Callback->>Trx: transmitFrame(code, encoder)
    loop 4 total frame emissions
        alt repeat frame
            Trx->>Enc: sendSilence()
        end
        Trx->>Trx: enableTransmitMode()
        Trx->>Chip: SIDLE / STX strobes
        Chip-->>Trx: MARCSTATE = TX
        Trx->>Streamer: streamFrameOnceStatic(code, encoder, firstFrame?)
        alt first frame
            Streamer->>Enc: sendPreamble()
        end
        Streamer->>Enc: send bits 0/1
        Streamer->>Enc: sendOpen()
        Enc->>Chip: toggle GDO0 async OOK data input
        Trx->>Chip: SIDLE
    end
    Callback-->>Loop: transmission complete
```
