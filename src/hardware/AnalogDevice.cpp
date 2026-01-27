#include <hardware/AnalogDevice.h>
#include <hardware/adc.h>

AnalogEvent::AnalogEvent(EventSource* source, uint8_t adc_pin, uint16_t adc_value)
    : Event(source), adc_value(adc_value)
{
}

AnalogDevice::AnalogDevice(uint8_t adc_pin, uint16_t adc_max_activation)
    : EventSource(), adc_pin(adc_pin), adc_max_activation(adc_max_activation)
{
    adc_pin = adc_pin > 28 ? 28 : adc_pin;
    adc_pin = adc_pin < 26 ? 26 : adc_pin;

    adc_gpio_init(adc_pin);
    adc_select_input(adc_pin - 26);
}

AnalogRepeatingDevice::AnalogRepeatingDevice(uint8_t adc_pin, uint16_t adc_max_activation, RepeatingTimer& repeat_timer, uint32_t window_ms, uint16_t adc_spacing_break)
    : AnalogDevice(adc_pin, adc_max_activation), repeat_timer(repeat_timer)
{
    init_data = new _InitData{break_detection_timer, latched_event};
    std::ignore = AddAction([](const Event* ev, void* ptr){
        _InitData* data = (_InitData*)ptr;
        data->break_detection_timer.Start(interval_ms);
        data->latched_event = (AnalogEvent*)ev; // set event when device is triggered
    }, init_data);

    timer_data = new _TimerData{repeat_timer, elapsed_ms, window_ms, adc_spacing_break, latched_event};
    // gets called every interval_ms milliseconds (defined in header file) until the ADC value changes more than the delta (adc_spacing_break)
    std::ignore = break_detection_timer.AddAction([](const Event* ev, void* ptr){
        _TimerData* data = (_TimerData*)ptr;
        RepeatingTimer* self = ev->GetSourceAsType<RepeatingTimer>();

        uint16_t adc_v_then = data->latched_event->GetADCValue();
        uint16_t adc_v_now = adc_read();

        // if the new adc value has a delta of 50 (device no longer active), end everything
        if (adc_v_now > adc_v_then + data->adc_spacing_break || adc_v_now < adc_v_then - data->adc_spacing_break)
        {
            self->End(); // should be called when the device goes inactive
            data->action_repeat_timer.End();
            data->elapsed_ms = 0;
        }
        
        // if enough time has passed without a changed adc value, begin the repeat until the adc value changes
        data->elapsed_ms += interval_ms;
        if (data->elapsed_ms >= data->wait_window_ms)
        {
            data->action_repeat_timer.Start();
        }

    }, timer_data);
}

AnalogRepeatingDevice::~AnalogRepeatingDevice()
{
    delete init_data;
    delete timer_data;
}