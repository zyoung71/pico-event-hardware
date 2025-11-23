#pragma once

#include "../event/Event.h"

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
    static void gpio_callback(uint8_t gpio_pin, uint32_t events_triggered_mask);

public:
    GPIODevice(uint8_t gpio_pin, Pull pull, uint32_t event_mask, void* user_data);
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