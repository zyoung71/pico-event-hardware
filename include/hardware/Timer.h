#pragma once

#include "../event/Event.h"

class TimerEvent : public Event
{
public:
    TimerEvent(EventSource* source);
};

class Timer : public EventSource
{
public:
    typedef TimerEvent EventType;

protected:
    absolute_time_t us_elapsed;
    absolute_time_t us_start;
    absolute_time_t us_begintime;

    uint32_t interval_ms; // only matters when the Start function is called without arguments

    bool is_active;

public:
    inline Timer(uint32_t interval_ms = 1000)
        : EventSource(), us_elapsed(0), us_start(0), us_begintime(0), is_active(false), interval_ms(interval_ms)
    {
    }
    virtual ~Timer() = default;

    virtual void Start(uint32_t ms) = 0; // overrides the initial timer interval
    inline void Start()
    {
        return Start(interval_ms);
    }
    virtual void End() = 0;

    inline uint64_t GetTimeElapsedMicroseconds() const
    {
        return us_elapsed;
    }
    inline uint64_t GetTimeElapsedMilliseconds() const
    {
        return us_elapsed / (uint64_t)1000;
    }
    inline float GetTimeElapsedSeconds() const
    {
        return us_elapsed * 1e-6f;
    }
    inline uint64_t GetTimeRemainingMicroseconds() const
    {
        return us_begintime - us_elapsed;
    }
    inline uint64_t GetTimeRemainingMilliseconds() const
    {
        return GetTimeRemainingMicroseconds() / (uint64_t)1000;
    }
    inline float GetTimeRemainingSeconds() const
    {
        return GetTimeElapsedMicroseconds() * 1e-6f;
    }
    inline void UpdateTime()
    {
        us_elapsed = to_us_since_boot(get_absolute_time()) - us_start;
    }
    inline bool IsActive() const
    {
        return is_active;
    }
};

class CountdownTimer : public Timer
{
private:
    static bool timer_callback(alarm_id_t id, void* timer);

protected:
    int id;

public:
    inline CountdownTimer(uint32_t interval_ms = 1000) : Timer(interval_ms), id(-1) {}
    virtual ~CountdownTimer();

    virtual void Start(uint32_t ms) override;
    virtual void End() override;

    using Timer::Start;
};

class RepeatingTimer : public Timer
{
private:
    static bool timer_callback(repeating_timer_t* timer);

protected:
    repeating_timer_t repeat_timer;

public:
    inline RepeatingTimer(uint32_t interval_ms = 1000) : Timer(interval_ms) {}
    virtual ~RepeatingTimer();

    virtual void Start(uint32_t ms) override;
    virtual void End() override;

    using Timer::Start;
};