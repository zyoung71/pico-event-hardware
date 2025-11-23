#pragma once

#include "../event/Event.h"

class Timer : public EventSource
{
protected:
    bool is_active;
    absolute_time_t us_elapsed;
    absolute_time_t us_start;
    absolute_time_t us_begintime;

public:
    inline Timer(void* user_data = nullptr)
        : EventSource(user_data), us_elapsed(0), us_start(0), us_begintime(0)
    {
    }
    virtual ~Timer() = default;

    virtual void Start(uint32_t ms) = 0;
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
    inline CountdownTimer(void* user_data = nullptr) : Timer(user_data), id(-1) {}
    virtual ~CountdownTimer();

    virtual void Start(uint32_t ms) override;
    virtual void End() override;
};

class RepeatingTimer : public Timer
{
private:
    static bool timer_callback(repeating_timer_t* timer);

protected:
    repeating_timer_t repeat_timer;

public:
    inline RepeatingTimer(void* user_data = nullptr) : Timer(user_data) {}
    virtual ~RepeatingTimer();

    virtual void Start(uint32_t ms) override;
    virtual void End() override;

};