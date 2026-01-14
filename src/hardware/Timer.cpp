#include <hardware/Timer.h>

TimerEvent::TimerEvent(EventSource* source)
    : Event(source)
{
}

bool CountdownTimer::timer_callback(alarm_id_t id, void* timer)
{
    CountdownTimer* self = (CountdownTimer*)timer;

    Event* ev = new TimerEvent(self);
    queue_try_add(&Event::event_queue, &ev);

    self->us_start = 0;
    self->End();
    return false;
}

CountdownTimer::~CountdownTimer()
{
    End();
}

void CountdownTimer::Start(uint32_t ms)
{
    if (!is_enabled)
        return;

    if (id >= 0 || is_active)
    {
        End();
        return Start(ms);
    }

    us_begintime = ms * 1000ULL;
    us_start = to_us_since_boot(get_absolute_time());
    id = add_alarm_in_ms(ms, (alarm_callback_t)&timer_callback, this, false);
}

void CountdownTimer::End()
{
    if (id >= 0 || is_active)
    {
        cancel_alarm(id);
        id = -1;
        is_active = false;
    }
}

bool RepeatingTimer::timer_callback(repeating_timer_t* timer)
{
    RepeatingTimer* self = (RepeatingTimer*)timer->user_data;

    Event* ev = new TimerEvent(self);
    queue_try_add(&Event::event_queue, &ev);
    
    self->us_start = to_us_since_boot(get_absolute_time());
    return true;
}

RepeatingTimer::~RepeatingTimer()
{
    End();
}

void RepeatingTimer::Start(uint32_t ms)
{
    if (!is_enabled)
        return;

    if (is_active)
    {
        End();
        return Start(ms);
    }

    us_begintime = ms * 1000ULL;
    us_start = to_us_since_boot(get_absolute_time());
    is_active = add_repeating_timer_ms(ms, (repeating_timer_callback_t)&timer_callback, this, &repeat_timer);
}

void RepeatingTimer::End()
{
    if (is_active)
    {
        cancel_repeating_timer(&repeat_timer);
        is_active = false;
    }
}