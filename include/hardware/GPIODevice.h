#pragma once

#include "../event/Event.h"

#include <util/Debounce.h>

#include <pico/stdlib.h>

enum class Pull
{
    NONE,
    UP,
    DOWN
};

class GPIODevice : public IRQSource
{
protected:
    static GPIODevice* instances[30];

    uint8_t gpio_pin;
    uint32_t event_mask;

    virtual void EnableImpl() override;
    virtual void DisableImpl() override;

    virtual void HandleIRQ(uint32_t events_triggered_mask) override;

private:
    static struct _GPIOEnableCallback
    {
        _GPIOEnableCallback();
    } _callback_enable_inst;

    static void gpio_callback(uint8_t gpio_pin, uint32_t events_triggered_mask);

public:
    GPIODevice(uint8_t gpio_pin, Pull pull, uint32_t event_mask);
    virtual ~GPIODevice();

    inline bool IsHigh() const
    {
        return gpio_get(gpio_pin);
    }
    inline bool IsLow() const
    {
        return !gpio_get(gpio_pin);
    }

    virtual bool IsActivated() const;
};

class GPIODeviceDebounce : public GPIODevice
{
protected:
    Debounce debouncer;

    virtual void HandleIRQ(uint32_t events_triggered_mask) override;

public:
    GPIODeviceDebounce(uint8_t gpio_pin, Pull pull, uint32_t event_mask, uint32_t debounce_ms);
    virtual ~GPIODeviceDebounce() = default;
};