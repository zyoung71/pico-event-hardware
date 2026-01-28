#pragma once

#include <event/Event.h>
#include "Timer.h"

class AnalogDevice;

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
public:
    typedef AnalogEvent EventType;

protected:
    uint8_t adc_pin;
    uint16_t adc_max_activation;

public:
    AnalogDevice(uint8_t adc_pin, uint16_t adc_max_activation);
    virtual ~AnalogDevice() = default;

    inline uint8_t GetADCPin() const
    {
        return adc_pin;
    }
    inline uint16_t GetADCActivation() const
    {
        return adc_max_activation;
    }
};

class AnalogRepeatingDevice : public AnalogDevice
{
private:
    static constexpr uint32_t interval_ms = 5;

    struct _InitData
    {
        RepeatingTimer& break_detection_timer;
        uint16_t& latched_adc;
    };
    struct _TimerData
    {
        RepeatingTimer& action_repeat_timer;
        uint32_t& elapsed_ms;
        uint32_t wait_window_ms;
        uint32_t adc_spacing_break;
        uint16_t& latched_adc;
    };

    _InitData* init_data;
    _TimerData* timer_data;

protected:
    RepeatingTimer break_detection_timer;
    RepeatingTimer& repeat_timer;

    uint32_t elapsed_ms = 0;
    uint16_t latched_adc_value = 0;

public:
    AnalogRepeatingDevice(uint8_t adc_pin, uint16_t adc_max_activation, RepeatingTimer& repeat_timer, uint32_t window_ms, uint16_t adc_spacing_break = 50);
    virtual ~AnalogRepeatingDevice();
};