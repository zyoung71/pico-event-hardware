#include <hardware/Button.h>

Button::Button(uint8_t gpio_pin, bool gnd_to_pin, uint32_t debounce_ms, void* user_data)
    : GPIODeviceDebounce(gpio_pin, gnd_to_pin ? Pull::UP : Pull::DOWN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, debounce_ms, user_data),
    gnd_to_pin(gnd_to_pin), release_actions_offset(0)
{
}

void Button::SetPressAndReleaseActions(CallbackAction* press_actions, size_t press_count,
    CallbackAction* release_actions, size_t release_count)
{
    event_actions = std::make_unique<CallbackAction[]>(press_count + release_count);
    if (press_actions)
        std::copy(press_actions, press_actions + press_count - 1, event_actions.get());
    if (release_actions)
        std::copy(release_actions, release_actions + release_count, event_actions.get() + press_count);
}

void Button::SetReleaseAndPressActions(CallbackAction* release_actions, size_t release_count,
    CallbackAction* press_actions, size_t press_count)
{
    SetPressAndReleaseActions(press_actions, press_count, release_actions, release_count);
}

void Button::HandleIRQ(uint32_t events_triggered_mask)
{
    if (events_triggered_mask & event_mask)
    {
        Event* ev = new ButtonEvent(this, events_triggered_mask);
        queue_try_add(&Event::event_queue, &ev);
    }
}

void Button::Dispatch(const Event* ev) const
{
    const ButtonEvent* self_event = (ButtonEvent*)ev;
    if (self_event->WasPressed())
    {
        for (size_t i = 0; i < release_actions_offset - 1; i++)
            event_actions[i](ev, user_data);
    }
    else
    {
        for (size_t i = release_actions_offset; i < action_count; i++)
            event_actions[i](ev, user_data);
    }
}

bool Button::IsActivated() const
{
    return gnd_to_pin ? !gpio_get(gpio_pin) : gpio_get(gpio_pin);
}

DoublePressButton::DoublePressButton(uint8_t gpio_pin, uint32_t window_ms, bool gnd_to_pin, uint32_t debounce_ms, void* user_data)
    : Button(gpio_pin, gnd_to_pin, debounce_ms, user_data), press_window_us(window_ms * 1000ULL), press_window_curr_time(to_us_since_boot(get_absolute_time()))
{
}

void DoublePressButton::SetDoublePressActions(CallbackAction* double_press_actions, size_t count)
{
    this->double_press_actions = std::make_unique<CallbackAction[]>(count);
    std::copy(double_press_actions, double_press_actions + count, this->double_press_actions.get());
}

void DoublePressButton::HandleIRQ(uint32_t events_triggeted_mask)
{
    if (event_mask & events_triggeted_mask)
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

TriplePressButton::TriplePressButton(uint8_t gpio_pin, uint32_t window_ms, bool gnd_to_pin, uint32_t debounce_ms, void* user_data)
    : DoublePressButton(gpio_pin, window_ms, gnd_to_pin, debounce_ms, user_data)
{
}

void TriplePressButton::SetTriplePressActions(CallbackAction* triple_press_actions, size_t count)
{
    this->triple_press_actions = std::make_unique<CallbackAction[]>(count);
    std::copy(triple_press_actions, triple_press_actions + count, this->triple_press_actions.get());
}

void TriplePressButton::HandleIRQ(uint32_t events_triggered_mask)
{
    if (event_mask & events_triggered_mask)
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

StickyButton::StickyButton(uint8_t gpio_pin, const GPIODevice* conditional_devices, size_t conditional_device_count, bool gnd_to_pin, uint32_t debounce_ms, void* user_data)
    : Button(gpio_pin, gnd_to_pin, debounce_ms, user_data), conditional_devices(conditional_devices), conditional_device_count(conditional_device_count)
{
}

void StickyButton::HandleIRQ(uint32_t events_triggered_mask)
{
    if (event_mask & events_triggered_mask)
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