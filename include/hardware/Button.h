#pragma once

#include "GPIODevice.h"

class Button : public GPIODeviceDebounce
{
protected:
    bool gnd_to_pin;

    virtual void HandleIRQ(uint32_t events_triggered_mask) override;

public:
    Button(uint8_t gpio_pin, bool gnd_to_pin = true, uint32_t debounce_ms = 50);
    virtual ~Button() = default;

    inline bool IsWiredToGround() const
    {
        return gnd_to_pin;
    }

    bool IsActivated() const override;
};

class DoublePressButton : public Button
{
protected:
    bool double_press_intermediate; // switched on when button is pressed once, awaiting the second

    absolute_time_t press_window_us;
    absolute_time_t press_window_curr_time;

    virtual void HandleIRQ(uint32_t events_triggered_mask) override;

public:
    DoublePressButton(uint8_t gpio_pin, uint32_t window_ms = 500, bool gnd_to_pin = true, uint32_t debounce_ms = 50);
    virtual ~DoublePressButton() = default;

    inline void SetPressTimeWindowMs(absolute_time_t window_ms)
    {
        press_window_us = window_ms * 1000ULL;
    }
};

class TriplePressButton : public DoublePressButton
{
protected:
    bool triple_press_intermediate; // switched on when button is pressed twice, awaiting the third

    virtual void HandleIRQ(uint32_t events_triggered_mask) override;

public:
    TriplePressButton(uint8_t gpio_pin, uint32_t window_ms = 500, bool gnd_to_pin = true, uint32_t debounce_ms = 50);
    virtual ~TriplePressButton() = default;
};

class StickyButton : public Button
{
protected:
    const GPIODevice* conditional_devices;
    size_t conditional_device_count;

    virtual void HandleIRQ(uint32_t events_triggered_mask) override;

public:
    StickyButton(uint8_t gpio_pin, const GPIODevice* conditional_devices, size_t conditional_device_count, bool gnd_to_pin = true, uint32_t debounce_ms = 50);

};