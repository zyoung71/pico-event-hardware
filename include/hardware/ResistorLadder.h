#pragma once

#include "AnalogDevice.h"
#include "GPIODevice.h"

#include <util/ArraySupplier.h>

#include <algorithm>
#include <hardware/adc.h>

// All devices attached must be on the same ADC pin. If not, it will use the first element's ADC pin for ALL devices.
// The devices attached must be sorted by their ADC activation value from lowest first to highest last. Call the SortDevices function if you are not certain of the ladder's order.
template<size_t ladder_bars>
class ResistorLadder : public GPIODeviceDebounce, public ArraySupplier<AnalogDevice*, ladder_bars>
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
    ResistorLadder(uint8_t shared_gpio_pin, Pull pull, uint32_t event_mask, uint16_t deadzone_max = 0xFFF, uint16_t deadzone_min = 0, uint32_t debounce_ms = 50)
        : GPIODeviceDebounce(shared_gpio_pin, pull, event_mask, debounce_ms), deadzone_max(deadzone_max), deadzone_min(deadzone_min)
    {
    }
    virtual ~ResistorLadder() = default;

    void SortDevices()
    {
        std::sort(this->array.begin(), this->array.end(), _ADCValueCompare);
    }

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
                adc_select_input(this->array[0]->GetADCPin() - 26);
                uint16_t adc_v = adc_read();
                if (adc_v < deadzone_min)
                    return;

                for (auto& source : this->array)
                {
                    if (!source->IsEnabled())
                        continue;

                    if (adc_v < source->GetADCActivation())
                    {
                        Event* ev_adc = new AnalogEvent(source, source->GetADCPin(), adc_v);
                        ProcessImmediateActions(ev_adc);
                        queue_try_add(&Event::event_queue, &ev_adc);
                        return;
                    }
                }

                //if (adc_v > deadzone_max) // Unnecessary for now.
                //    return;
            }
        }
    }
};