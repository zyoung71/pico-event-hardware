#include <hardware/Button.h>

Button::Button(uint8_t gpio_pin, bool gnd_to_pin, uint32_t debounce_ms)
    : GPIODeviceDebounce(gpio_pin, gnd_to_pin ? Pull::UP : Pull::DOWN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, debounce_ms),
    gnd_to_pin(gnd_to_pin)
{
}

void Button::HandleIRQ(uint32_t events_triggered_mask)
{
    if (events_triggered_mask & event_mask)
    {
        if (debouncer.Allow())
        {
            Event* ev = new ButtonEvent(this, events_triggered_mask);
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}

bool Button::IsActivated() const
{
    return gnd_to_pin ? !gpio_get(gpio_pin) : gpio_get(gpio_pin);
}

DoublePressButton::DoublePressButton(uint8_t gpio_pin, uint32_t window_ms, bool gnd_to_pin, uint32_t debounce_ms)
    : Button(gpio_pin, gnd_to_pin, debounce_ms), press_window_us(window_ms * 1000ULL), press_window_curr_time(to_us_since_boot(get_absolute_time()))
{
}

void DoublePressButton::HandleIRQ(uint32_t events_triggeted_mask)
{
    if (event_mask & events_triggeted_mask)
    {
        if (debouncer.Allow())
        {
            absolute_time_t now = to_us_since_boot(get_absolute_time());
            if (press_window_curr_time - now >= press_window_us)
            {
                double_press_intermediate = false;
            }
    
            if (double_press_intermediate)
            {
                double_press_intermediate = false;
                Event* ev = new ButtonEvent(this, events_triggeted_mask, 2);
                queue_try_add(&Event::event_queue, &ev);
            }
            else
            {
                double_press_intermediate = true;
                Event* ev = new ButtonEvent(this, events_triggeted_mask, 1);
                queue_try_add(&Event::event_queue, &ev);
            }
    
            press_window_curr_time = to_us_since_boot(get_absolute_time());
        }
    }
}

TriplePressButton::TriplePressButton(uint8_t gpio_pin, uint32_t window_ms, bool gnd_to_pin, uint32_t debounce_ms)
    : DoublePressButton(gpio_pin, window_ms, gnd_to_pin, debounce_ms)
{
}

void TriplePressButton::HandleIRQ(uint32_t events_triggered_mask)
{
    if (event_mask & events_triggered_mask)
    {
        if (debouncer.Allow())
        {
            absolute_time_t now = to_us_since_boot(get_absolute_time());
            if (press_window_curr_time - now >= press_window_us)
            {
                triple_press_intermediate = false;
                double_press_intermediate = false;
            }
            
            if (triple_press_intermediate) // On third press
            {
                triple_press_intermediate = false;
                double_press_intermediate = false;
                Event* ev = new ButtonEvent(this, events_triggered_mask, 3);
                queue_try_add(&Event::event_queue, &ev);
            }
            else if (double_press_intermediate) // On second press
            {
                triple_press_intermediate = true;
                Event* ev = new ButtonEvent(this, events_triggered_mask, 2);
                queue_try_add(&Event::event_queue, &ev);
            }
            else // On first press
            {
                double_press_intermediate = true;
                Event* ev = new ButtonEvent(this, events_triggered_mask, 1);
                queue_try_add(&Event::event_queue, &ev);
            }
    
            press_window_curr_time = to_us_since_boot(get_absolute_time());
        }
    }
}

StickyButton::StickyButton(uint8_t gpio_pin, const GPIODevice* conditional_devices, size_t conditional_device_count, bool gnd_to_pin, uint32_t debounce_ms)
    : Button(gpio_pin, gnd_to_pin, debounce_ms), conditional_devices(conditional_devices), conditional_device_count(conditional_device_count)
{
}

void StickyButton::HandleIRQ(uint32_t events_triggered_mask)
{
    if (event_mask & events_triggered_mask)
    {
        if (debouncer.Allow())
        {
            for (size_t i = 0; i < conditional_device_count; i++)
            {
                if (!conditional_devices[i].IsActivated())
                    return;
            }
    
            Event* ev = new ButtonEvent(this, events_triggered_mask);
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}