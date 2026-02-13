#include <hardware/GPIODevice.h>

GPIOEvent::GPIOEvent(EventSource* source, uint32_t events_triggered_mask)
    : Event(source), events_triggered_mask(events_triggered_mask)
{
}

GPIODevice::_GPIOEnableCallback::_GPIOEnableCallback()
{
    gpio_set_irq_callback((gpio_irq_callback_t)&gpio_callback);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

GPIODevice::_GPIOEnableCallback GPIODevice::_callback_enable_inst = GPIODevice::_GPIOEnableCallback();

GPIODevice* GPIODevice::instances[48] = {};

void GPIODevice::gpio_callback(uint8_t gpio_pin, uint32_t events_triggered_mask)
{
    if (instances[gpio_pin])
    {
        instances[gpio_pin]->HandleIRQ(events_triggered_mask);
    }
}

GPIODevice::GPIODevice(uint8_t gpio_pin, Pull pull, uint32_t event_mask)
    : IRQSource(), gpio_pin(gpio_pin), event_mask(event_mask)
{
    gpio_pin = gpio_pin > 30 ? 30 : gpio_pin;
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

GPIODevice::~GPIODevice()
{
    instances[gpio_pin] = nullptr;
}

void GPIODevice::EnableImpl()
{
    gpio_set_irq_enabled(gpio_pin, event_mask, true);
}

void GPIODevice::DisableImpl()
{
    gpio_set_irq_enabled(gpio_pin, event_mask, false);
}

void GPIODevice::HandleIRQ(uint32_t events_triggered_mask)
{
    if (event_mask & events_triggered_mask)
    {
        Event* ev = new GPIOEvent(this, events_triggered_mask);
        ProcessImmediateActions(ev);
        queue_try_add(&Event::event_queue, &ev);
    }
}

bool GPIODevice::IsActivated() const
{
    return gpio_get(gpio_pin);
}

GPIODeviceDebounce::GPIODeviceDebounce(uint8_t gpio_pin, Pull pull, uint32_t event_mask, uint32_t debounce_ms)
    : GPIODevice(gpio_pin, pull, event_mask), debouncer(debounce_ms)
{
}

void GPIODeviceDebounce::HandleIRQ(uint32_t events_triggered_mask)
{
    if (event_mask & events_triggered_mask)
    {
        if (debouncer.Allow())
        {
            Event* ev = new GPIOEvent(this, events_triggered_mask); 
            ProcessImmediateActions(ev);  
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}