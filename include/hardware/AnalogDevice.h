#pragma once

#include <event/Event.h>

class AnalogEvent : public Event
{
protected:
    uint8_t adc_pin;
    uint16_t adc_value;

public:
    AnalogEvent(EventSource* source, uint8_t adc_pin, uint16_t adc_value);

    inline uint8_t GetADCPin() const
    {
        return adc_pin;
    }
    inline uint16_t GetADCValue() const
    {
        return adc_value;
    }
};

class AnalogDevice : public EventSource
{
protected:
    uint8_t adc_pin;
    uint16_t adc_min_activation;

public:
    AnalogDevice(uint8_t adc_pin, uint16_t adc_min_activation);
    virtual ~AnalogDevice() = default;

    inline uint8_t GetADCPin() const
    {
        return adc_pin;
    }
    inline uint16_t GetADCActivation() const
    {
        return adc_min_activation;
    }
};