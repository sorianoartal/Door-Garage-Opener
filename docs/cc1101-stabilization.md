# CC1101 Stabilization And Debugging History

This note records the main issues found while stabilizing the CC1101 bring-up path and explains why each one mattered.

## Summary

The project was very close to working, but several small issues stacked together:

- one table-size bug,
- one pin-ownership bug,
- one SPI transaction timing gap,
- one invalid read-validity rule,
- one verification-layer design problem,
- and weak logs that made the failures look more random than they really were.

## What Was Wrong

### 1. The register-table size constant did not match the table contents

Problem:

- `NUM_REG_TO_CONFIG_CC1101` was set to `24`.
- The 315 MHz OOK configuration table actually contained `23` real entries.
- Because the table is a `std::array`, the extra slot became zero-initialized.

Impact:

- Bring-up wrote one unintended extra register entry.
- That made the startup sequence harder to reason about and could silently alter radio state.

Fix:

- The constant was changed to `23` so the table length matches the real data.

### 2. The MCU claimed the `GDO0` line too early

Problem:

- `encoder.begin()` originally ran before the transceiver had finished configuring the CC1101.
- The project uses `GDO0` as the async data input, so that line is shared between the MCU output logic and the radio's pin behavior.

Impact:

- The first CC1101 register touching `GDO0` could fail readback or behave inconsistently.
- This showed up as unstable `IOCFG0` verification.

Fix:

- `encoder.begin()` now runs only after `transceiver.begin()` succeeds.

### 3. A valid register value of `0xFF` was incorrectly treated as a failed read

Problem:

- `ReadResult::isValid()` originally required both `status != 0xFF` and `value != 0xFF`.
- That is wrong because `0xFF` can be a legitimate CC1101 register value.
- `PKTLEN = 0xFF` is normal in this project.

Impact:

- Reads of `PKTLEN` were always marked invalid even when the bus transaction succeeded.
- This caused false retries and false configuration failure reports.

Fix:

- Read validity is now based on the status byte only.
- `0xFF` is allowed as a legal register value.

### 4. The low-level SPI write path was verifying writes even when the caller asked not to

Problem:

- `SPIBus::writeRegister()` always read the register back immediately.
- That ignored the higher-level `VERIFY` / `SKIP_VERIFY` decision in the config helper.

Impact:

- Registers meant to skip verification were still effectively verified.
- This made the register policy misleading and amplified failures on marginal reads.

Fix:

- `SPIBus::writeRegister()` now only performs the write transport.
- Verification is handled by `applyRegisterConfig_CC1101()` where the policy actually lives.

### 5. Normal SPI transactions did not wait for the CC1101 ready signal

Problem:

- The manual reset sequence waited for `MISO` to go low before sending `SRES`.
- Regular SPI register reads and writes did not wait for that ready condition after asserting `CSn`.

Impact:

- Some accesses were started before the CC1101 was ready.
- This likely caused the intermittent wrong readbacks seen on `IOCFG0` and `FIFOTHR`.

Fix:

- The shared SPI transaction wrapper now waits for the CC1101 ready signal on each normal transaction.

### 6. Configuration verification was too brittle for noisy bring-up

Problem:

- A single mismatched readback immediately failed configuration.
- During stabilization, some reads were correct on the next attempt.

Impact:

- Transient readback noise could abort the full startup even when the write itself succeeded.

Fix:

- Readback verification now retries a few times before failing.

### 7. The logs were too fragmented to support efficient debugging

Problem:

- Old logs spread one logical event across several lines.
- Register names were often missing.
- Retry output did not always make it obvious whether the failure was transport, timing, or policy.

Impact:

- Failures looked more chaotic than they were.
- It was harder to see that `PKTLEN = 0xFF` was a false-negative validation bug and not a bus failure.

Fix:

- Logging now prints concise structured lines such as:

```text
[SPI][WRITE][RETRY 1/3] IOCFG0 (0x02): expected 0x2E, read 0x2A, status 0x00
[CC1101][CFG][ERROR] Verify failed for FIFOTHR (0x03) after 3 attempts, expected 0x07
```

## Why The System Works Better Now

The bring-up path is now more deterministic because:

- the table length is correct,
- the shared `GDO0` line is claimed in the right order,
- valid `0xFF` register values are no longer rejected,
- verification only happens where intended,
- SPI timing respects the CC1101 ready handshake,
- and the debug output points to the real failing layer when something goes wrong.

## Lessons Learned

### Keep transport and verification separate

The SPI layer should move bytes reliably.

The configuration helper should decide whether a specific register must be verified.

### Shared pins need ownership rules

When a pin is used by both the MCU and the transceiver, initialization order is part of the design.

### A diagnostic rule can be as dangerous as a logic bug

The `0xFF` validity check looked harmless, but it created a false story about the health of the SPI bus.

### Human-readable logs save time

For radio bring-up, the best logs usually show:

- what register was targeted,
- what value was expected,
- what value came back,
- whether the chip was ready,
- and which attempt failed.
