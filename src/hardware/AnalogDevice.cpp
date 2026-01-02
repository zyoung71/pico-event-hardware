#include <hardware/AnalogDevice.h>
#include <hardware/adc.h>

AnalogEvent::AnalogEvent(const EventSource* source, uint8_t adc_pin, uint16_t adc_value)
    : Event(source), adc_value(adc_value)
{
}

AnalogDevice::AnalogDevice(uint8_t adc_pin, uint16_t adc_min_activation)
    : EventSource(), adc_pin(adc_pin), adc_min_activation(adc_min_activation)
{
    adc_pin = adc_pin > 28 ? 28 : adc_pin;
    adc_pin = adc_pin < 26 ? 26 : adc_pin;

    adc_gpio_init(adc_pin);
}