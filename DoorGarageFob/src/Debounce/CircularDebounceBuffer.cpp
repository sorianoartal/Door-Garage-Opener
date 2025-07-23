#include"Debounce/CircularDebounceBuffer.h"
#include <Arduino.h>


/**
 * Constructor
 *
 * @param id              An arbitrary identifier (passed to callbacks in other designs,  but unused here since callbacks have no arguments)
 * @param pin             The digital pin to read (wired either active‐LOW or active‐HIGH)
 * @param isActiveLow     If true, a LOW reading means “pressed”
 * @param delayBetweenUs  How many microseconds between internal samples (e.g. 1000 for 1 ms)
 */
CircularDebounceBuffer::CircularDebounceBuffer(
    uint8_t id,
    uint8_t pin,
    bool    isActiveLow,
    uint32_t delayBetweenUs
)
: _pin_ID(id),
  _pin(pin),
  _isActiveLow(isActiveLow),
  _debouncing(false),
  _stableState(false),
  _pressedDetected(false),
  _head(0),
  _thresholdPercentage(90),                         // default to 90% (you can override in setup)
  _callbackCounter(0),
  _delayBetweenSamples(delayBetweenUs)  // initialize Delay with desired interval
{
    clearBuffer();
}

/**
 * Clear the entire sample buffer (set all slots to false) and reset head to 0.
 */
void CircularDebounceBuffer::clearBuffer()
{
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        _buffer[i] = false;
    }
    _head = 0;
}

/**
 * Set how many percent of BUFFER_SIZE must be “true” to confirm a press.
 * Example: BUFFER_SIZE = 16, percentage = 60 → thresholdCount = ceil(16 * 60/100) = 10.
 */
void CircularDebounceBuffer::setThreshold(uint8_t percentage)
{
    if (percentage <= 100) {
        _thresholdPercentage = percentage;
    }
}

/**
 * Adjust the sample interval (in microseconds).
 * Delay::updateDelayTime simply changes the internal target time.
 */
void CircularDebounceBuffer::setSampleIntervalUs(uint32_t newDelayUs)
{
    _delayBetweenSamples.updateDelayTime(newDelayUs);
}

/**
 * Register a zero‐argument callback. Up to MAX_CALLBACKS can be stored.
 */
void CircularDebounceBuffer::addCallback(Callback cb)
{
    if (_callbackCounter < MAX_CALLBACKS) {
        _callbacks[_callbackCounter++] = cb;
    }
}

/**
 * Called from your raw FALLING‐edge ISR. Arms the debouncer for a new press.
 * It will clear the buffer, reset flags, and start the Delay countdown.
 */
void CircularDebounceBuffer::startDebounce()
{
    // Only arm if we’re not already debouncing AND the last stable state is “not pressed.”
    if (!_debouncing && !_stableState) {
        _debouncing      = true;
        _pressedDetected = false;                   // allow the next “press” to fire a callback
        clearBuffer();                                      // drop any old samples
        _delayBetweenSamples.restartTimer();  // begin counting from now
    }
}

/**
 * Must be called repeatedly from loop(). It will:
 *  1) Check if we are currently debouncing. If not, return immediately.
 *  2) Call isDelayTimeElapsed(). If < interval has passed, return.
 *  3) Once >= interval passes, Delay automatically restarts itself internally.
 *  4) Read the pin, normalize, and insert into the buffer at _head.
 *  5) Count how many “true” bits are in the buffer.
 *  6) If we haven’t yet fired a “pressed” callback this session and trueCount ≥ threshold, fire it.
 *  7) If we have fired a press, wait until trueCount ≤ (BUFFER_SIZE – threshold) to confirm release, then disarm.
 *  8) Otherwise, keep sampling next time.
 */
void CircularDebounceBuffer::update()
{
    // 1) If not currently in a debouncing session, do nothing
    if (!_debouncing) {
        return;
    }

    // 2) Only take a new sample if the Delay interval has just elapsed.
    //    Delay::isDelayTimeElapsed() returns true ONCE when micros() - previousTime >= delayTime,
    //    and internally calls restartTimer() immediately. So we do not call restartTimer() here.
    if (!_delayBetweenSamples.isDelayTimeElapsed()) {
        return;
    }

    // 3) Now we know a full sample interval has passed. The Delay object
    //    already called restartTimer() inside isDelayTimeElapsed().
    //    We can safely proceed to read the pin and insert into buffer.

    // 4) Read raw pin, normalize for active‐LOW/high, and write into buffer
    bool raw      = digitalRead(_pin);
    bool adjusted = _isActiveLow ? !raw : raw;
    _buffer[_head] = adjusted;
    _head = (size_t)((_head + 1) % BUFFER_SIZE);

    // 5) Count how many “true” bits are in the buffer now
    uint8_t trueCount = 0;
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        if (_buffer[i]) {
            ++trueCount;
        }
    }

    // 6) Compute the absolute number of “true” samples needed:
    //    thresholdCount = ceiling(BUFFER_SIZE * thresholdPercentage / 100).
    uint8_t thresholdCount = (BUFFER_SIZE * _thresholdPercentage + 99) / 100;

    // ======================================================
    // A) If we have not yet fired “pressed” in this session (_pressedDetected == false):
    if (!_pressedDetected) {
        if (trueCount >= thresholdCount) {
            // We have reached enough consecutive “true” samples → confirm “pressed”
            _stableState     = true;   // remember we’re pressed now
            _pressedDetected = true;   // so we won’t fire again until release

            // Fire all registered zero‐arg callbacks exactly once
            for (size_t i = 0; i < _callbackCounter; ++i) {
                if (_callbacks[i]) {
                    _callbacks[i]();
                }
            }

            // We do NOT clear or disarm yet; we stay debouncing
            // so that we can detect the release later.
        }
        // Return immediately; wait until the next sample interval to proceed.
        return;
    }

    // ======================================================
    // B) If a press has already been detected (_pressedDetected == true):
    //    Wait until we see enough “false” samples to confirm a release.
    {
        // “Released” consensus means trueCount <= (BUFFER_SIZE - thresholdCount)
        if (trueCount <= (BUFFER_SIZE - thresholdCount)) {
            // Confirmed release:
            _stableState     = false;  // store debounced state = “released”
            _pressedDetected = false;  // allow the next press to fire

            clearBuffer();   // drop old data
            _debouncing = false; // disarm until next raw FALLING
        }
        return;
    }
    // ======================================================
    // C) If we get here, it means trueCount is between thresholdCount and
    //    (BUFFER_SIZE - thresholdCount). Still bouncing, so just wait.
    // ======================================================
}

bool CircularDebounceBuffer::getStableState() const
{
    return _stableState;
}

/**
 * Completely re‐initialize:
 *  • Clear the buffer
 *  • Reset flags (_stableState, _pressedDetected, _debouncing)
 *  • Reset callback counter
 *  • Restart the internal Delay timer
 */
void CircularDebounceBuffer::reset()
{
    clearBuffer();
    _stableState     = false;
    _pressedDetected = false;
    _debouncing      = false;
    _callbackCounter = 0;
}