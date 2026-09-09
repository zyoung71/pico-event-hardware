#pragma once

#include <cstdint>

class PulseWidthModulation
{
protected:
    float clock;
    uint16_t wrap;
    uint8_t gpio_pin;
    uint8_t slice;
    uint8_t channel;

public:
    PulseWidthModulation(uint8_t gpio_pin, float frequency, float duty_cycle);
    PulseWidthModulation(uint8_t gpio_pin);

    void SetFrequency(float frequency);
    void SetDutyCycle(float duty_cycle);
};