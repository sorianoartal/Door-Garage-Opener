#pragma once

#include <Arduino.h>

class DigitalPin {
private:
    char port;
    uint8_t pin;

public:

    DigitalPin(char port, uint8_t pin) : port(port), pin(pin) {}
    ~DigitalPin() = default;

    inline void pinConfig(bool input, bool pullup) {
        if (input) {
            switch (port) {
                case 'B':
                    DDRB &= ~(1 << pin);
                    if (pullup) PORTB |= (1 << pin);
                    break;
                case 'C':
                    DDRC &= ~(1 << pin);
                    if (pullup) PORTC |= (1 << pin);
                    break;
                case 'D':
                    DDRD &= ~(1 << pin);
                    if (pullup) PORTD |= (1 << pin);
                    break;
                default:
                    break;
            }
        } else {
            switch (port) {
                case 'B': DDRB |= (1 << pin); break;
                case 'C': DDRC |= (1 << pin); break;
                case 'D': DDRD |= (1 << pin); break;
                default: break;
            }
        }
    }

    inline void writePin(uint8_t data) {
        switch (port) {
            case 'B':
                if (data) PORTB |= (1 << pin);
                else PORTB &= ~(1 << pin);
                break;
            case 'C':
                if (data) PORTC |= (1 << pin);
                else PORTC &= ~(1 << pin);
                break;
            case 'D':
                if (data) PORTD |= (1 << pin);
                else PORTD &= ~(1 << pin);
                break;
            default:
                break;
        }
    }

    inline uint8_t readPin() {
        switch (port) {
            case 'B': return (PINB & (1 << pin)) ? HIGH : LOW;
            case 'C': return (PINC & (1 << pin)) ? HIGH : LOW;
            case 'D': return (PIND & (1 << pin)) ? HIGH : LOW;
            default: return 0;
        }
    }
};