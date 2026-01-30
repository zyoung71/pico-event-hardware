#include <hardware/GPIODevice.h>

GPIOEvent::GPIOEvent(EventSourceBase* source, uint32_t events_triggered_mask)
    : Event(source), events_triggered_mask(events_triggered_mask)
{
}

GPIODeviceBase::_GPIOEnableCallback::_GPIOEnableCallback()
{
    gpio_set_irq_callback((gpio_irq_callback_t)&gpio_callback);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

GPIODeviceBase::_GPIOEnableCallback GPIODeviceBase::_callback_enable_inst;

GPIODeviceBase* GPIODeviceBase::instances[48] = {};