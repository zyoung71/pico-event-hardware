#pragma once

#include "AnalogDevice.h"
#include "GPIODevice.h"

#include <util/ArraySupplier.h>

#include <algorithm>
#include <hardware/adc.h>

#define ADC_PIN_BEGIN_OFFSET 26

template<size_t ladder_bars>
class ResistorLadder : public ArraySupplier<AnalogDevice*, ladder_bars>
{
private:
    static bool _ADCValueCompare(const AnalogDevice* a, const AnalogDevice* b)
    {
        return a->GetADCActivation() < b->GetADCActivation();
    }

protected:
    uint16_t deadzone_max;
    uint16_t deadzone_min;

public:
    ResistorLadder(uint16_t deadzone_min = 0, uint16_t deadzone_max = 0xFFF)
        : deadzone_min(deadzone_min), deadzone_max(deadzone_max) {}
    virtual ~ResistorLadder() = default;

    void SortDevices()
    {
        std::sort(this->array.begin(), this->array.end(), _ADCValueCompare);
    }

    void DetectADC()
    {
        adc_select_input(this->array[0]->GetADCPin() - ADC_PIN_BEGIN_OFFSET);
        uint16_t adc_v = adc_read();

        if (adc_v < deadzone_min || adc_v > deadzone_max)
            return;
    
        for (AnalogDevice* source : this->array)
        {
            if (!source->IsEnabled())
                continue;
        
            if (adc_v < source->GetADCActivation())
            {
                Event* ev_adc = new AnalogEvent(source, source->GetADCPin(), adc_v);
                source->ProcessImmediateActions(ev_adc);
                queue_try_add(&Event::event_queue, &ev_adc);
                return;
            }
        }
    }

    void DetectADCAndLatch()
    {
        static int32_t previous_button_index = -1;
        int32_t current_button_index;

        adc_select_input(this->array[0]->GetADCPin() - ADC_PIN_BEGIN_OFFSET);
        uint16_t adc_v = adc_read();

        if (adc_v < deadzone_min || adc_v > deadzone_max)
            return;

        for (size_t i = 0; i < ladder_bars; i++)
        {
            AnalogDevice* source = this->array[i];

            if (!source->IsEnabled())
                continue;

            if (adc_v < source->GetADCActivation())
            {
                current_button_index = i;
                if (current_button_index == previous_button_index)
                {
                    return; // cancel
                }
                previous_button_index = current_button_index;
                Event* ev_adc = new AnalogEvent(source, source->GetADCPin(), adc_v);
                source->ProcessImmediateActions(ev_adc);
                queue_try_add(&Event::event_queue, &ev_adc);
                return;
            }
            previous_button_index = -1; // if no button pressed
        }
    }

};

// All devices attached must be on the same ADC pin. If not, it will use the first element's ADC pin for ALL devices.
// The devices attached must be sorted by their ADC activation value from lowest first to highest last. Call the SortDevices function if you are not certain of the ladder's order.
template<size_t ladder_bars>
class DetectableResistorLadder : public GPIODeviceDebounce, public ResistorLadder<ladder_bars>
{
public:
    DetectableResistorLadder(uint8_t shared_gpio_pin, Pull pull, uint32_t event_mask, uint16_t deadzone_min = 0, uint16_t deadzone_max = 0xFFF, uint32_t debounce_ms = 50)
        : GPIODeviceDebounce(shared_gpio_pin, pull, event_mask, debounce_ms), ResistorLadder<ladder_bars>(deadzone_min, deadzone_max)
    {
    }
    virtual ~DetectableResistorLadder() = default;

    virtual void HandleIRQ(uint32_t events_triggered_mask) override
    {
        if (event_mask & events_triggered_mask)
        {
            if (debouncer.Allow())
            {
                if (event_actions.size() > 0)
                {
                    Event* ev_gpio = new GPIOEvent(this, events_triggered_mask);
                    queue_try_add(&Event::event_queue, &ev_gpio);
                }   
                this->DetectADC();
            }
        }
    }
};