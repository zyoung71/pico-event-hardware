#include <hardware/Button.h>
#include <hardware/Timer.h>

Button::Button(uint8_t gpio_pin, bool gnd_to_pin, void* user_data)
    : GPIODevice(gpio_pin, gnd_to_pin ? Pull::UP : Pull::DOWN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, user_data),
    gnd_to_pin(gnd_to_pin)
{
}

void Button::SetPressAndReleaseActions(CallbackAction* press_actions, size_t press_count,
    CallbackAction* release_actions, size_t release_count)
{
    all_actions = std::make_unique<CallbackAction[]>(press_count + release_count);
    if (press_actions)
        std::copy(press_actions, press_actions + press_count - 1, all_actions.get());
    if (release_actions)
        std::copy(release_actions, release_actions + release_count, all_actions.get() + press_count);
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

bool Button::IsActivated() const
{
    return gnd_to_pin ? !gpio_get(gpio_pin) : gpio_get(gpio_pin);
}

void DoublePressButton::timer_action(const Event* ev, void* user_data)
{
    DoublePressButton* self = (DoublePressButton*)user_data;
    self->double_press_intermediate = false;
}

DoublePressButton::DoublePressButton(uint8_t gpio_pin, absolute_time_t window_ms, bool gnd_to_pin, void* user_data)
    : Button(gpio_pin, gnd_to_pin, user_data), press_window_ms(window_ms), press_window_timer(CountdownTimer(this))
{
    static auto func_ptr = &timer_action;
    press_window_timer.SetActions(&func_ptr, 1);
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
        if (double_press_intermediate)
        {
            press_window_timer.End();
            Event* ev = new ButtonEvent(this, events_triggeted_mask, 2);
            queue_try_add(&Event::event_queue, &ev);
        }
        else
        {
            Event* ev = new ButtonEvent(this, events_triggeted_mask, 1);
            queue_try_add(&Event::event_queue, &ev);
            double_press_intermediate = true;
            press_window_timer.Start(press_window_ms);
        }
    }
}

void TriplePressButton::timer_action(const Event* ev, void* user_data)
{
    TriplePressButton* self = (TriplePressButton*)user_data;
    self->triple_press_intermediate = false;
    self->double_press_intermediate = false;
}

TriplePressButton::TriplePressButton(uint8_t gpio_pin, absolute_time_t window_ms, bool gnd_to_pin, void* user_data)
    : DoublePressButton(gpio_pin, window_ms, gnd_to_pin, user_data)
{
    static auto func_ptr = &timer_action;
    press_window_timer.SetActions(&func_ptr, 1);
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
        if (triple_press_intermediate) // On third press
        {
            press_window_timer.End();
            Event* ev = new ButtonEvent(this, events_triggered_mask, 3);
            queue_try_add(&Event::event_queue, &ev);
        }
        else if (double_press_intermediate) // On second press
        {
            Event* ev = new ButtonEvent(this, events_triggered_mask, 2);
            queue_try_add(&Event::event_queue, &ev);
            triple_press_intermediate = true;
            press_window_timer.Start(press_window_ms);
        }
        else // On first press
        {
            Event* ev = new ButtonEvent(this, events_triggered_mask, 1);
            queue_try_add(&Event::event_queue, &ev);
            double_press_intermediate = true;
            press_window_timer.Start(press_window_ms);
        }
    }
}

StickyButton::StickyButton(uint8_t gpio_pin, const GPIODevice* conditional_devices, size_t conditional_device_count, bool gnd_to_pin, void* user_data)
    : Button(gpio_pin, gnd_to_pin, user_data), conditional_devices(conditional_devices), conditional_device_count(conditional_device_count)
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