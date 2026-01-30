#pragma once

#include "../event/Event.h"

#include <util/Debounce.h>

#include <pico/stdlib.h>

class GPIOEvent : public Event
{
protected:
    uint32_t events_triggered_mask;

public:
    GPIOEvent(EventSourceBase* source, uint32_t events_triggered_mask);

    inline uint32_t GetEventsTriggeredMask() const
    {
        return events_triggered_mask;
    }
};

enum class Pull
{
    NONE,
    UP,
    DOWN
};

class GPIODeviceBase
{
protected:
    static GPIODeviceBase* instances[48];

    virtual void HandleIRQ(uint32_t events_triggered_mask) = 0; // match IRQSource's signature without inheriting

    static struct _GPIOEnableCallback
    {
        _GPIOEnableCallback();
    } _callback_enable_inst;

    static void gpio_callback(uint8_t gpio_pin, uint32_t events_triggered_mask)
    {
        if (instances[gpio_pin])
        {
            instances[gpio_pin]->HandleIRQ(events_triggered_mask);
        }
    }

};

template<class TEventType = GPIOEvent> requires std::is_base_of_v<GPIOEvent, TEventType>
class GPIODevice : public IRQSource<TEventType>, protected GPIODeviceBase
{
protected:
    uint8_t gpio_pin;
    uint32_t event_mask;

    virtual void EnableImpl() override
    {
        gpio_set_irq_enabled(gpio_pin, event_mask, true);
    }
    virtual void DisableImpl() override
    {
        gpio_set_irq_enabled(gpio_pin, event_mask, false);
    }

    virtual void HandleIRQ(uint32_t events_triggered_mask) override
    {
        if (event_mask & events_triggered_mask)
        {
            Event* ev = new GPIOEvent(this, events_triggered_mask);   
            queue_try_add(&Event::event_queue, &ev);
        }
    }

public:
    GPIODevice(uint8_t gpio_pin, Pull pull, uint32_t event_mask)
        : IRQSource<TEventType>(), gpio_pin(gpio_pin), event_mask(event_mask)
    {
        gpio_pin = gpio_pin > 47 ? 47 : gpio_pin;
        instances[gpio_pin] = this;

        gpio_init(gpio_pin);
        gpio_set_dir(gpio_pin, GPIO_IN);
        switch (pull)
        {
            case Pull::UP: gpio_pull_up(gpio_pin); break;
            case Pull::DOWN: gpio_pull_down(gpio_pin); break;
            default: gpio_disable_pulls(gpio_pin); break;
        }

        gpio_set_irq_enabled(gpio_pin, event_mask, true);
    }
    virtual ~GPIODevice()
    {
        instances[gpio_pin] = nullptr;
    }

    inline bool IsHigh() const
    {
        return gpio_get(gpio_pin);
    }
    inline bool IsLow() const
    {
        return !gpio_get(gpio_pin);
    }

    virtual bool IsActivated() const
    {
        return gpio_get(gpio_pin);
    }
};

template<class TEventType = GPIOEvent> requires std::is_base_of_v<GPIOEvent, TEventType>
class GPIODeviceDebounce : public GPIODevice<TEventType>
{
protected:
    Debounce debouncer;

    virtual void HandleIRQ(uint32_t events_triggered_mask) override
    {
        if (this->event_mask & events_triggered_mask)
        {
            if (debouncer.Allow())
            {
                Event* ev = new GPIOEvent(this, events_triggered_mask);   
                queue_try_add(&Event::event_queue, &ev);
            }
        }
    }

public:
    GPIODeviceDebounce(uint8_t gpio_pin, Pull pull, uint32_t event_mask, uint32_t debounce_ms)
        : GPIODevice<TEventType>(gpio_pin, pull, event_mask), debouncer(debounce_ms) {}
    virtual ~GPIODeviceDebounce() = default;
};