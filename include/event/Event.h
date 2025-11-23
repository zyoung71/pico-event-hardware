#pragma once

#include <cstdint>
#include <memory>

#include <pico/util/queue.h>

#include <util/ResultOptional.h>

class EventSource;

class Event
{
private:
    static bool is_queue_init;

protected:
    const EventSource* source;

public:
    static queue_t event_queue;

    static void HandleEvents();

    Event(const EventSource* source);

    /**
     * @return An optional containing the downcasted event if successful, and the generic event if not.
     * 
    */
    template<class EventTemplate>
    ResultOptional<const EventTemplate*, bool> GetEventAsType() const
    {
        if (EventTemplate* ev = (EventTemplate*)this)
            return ResultOptional(ev, true);
        return ResultOptional(this, false);
    }
};

class GPIOEvent : public Event
{
protected:
    uint32_t events_triggered_mask;

public:
    GPIOEvent(const EventSource* source, uint32_t events_triggered_mask);

    inline uint32_t GetEventsTriggeredMask() const
    {
        return events_triggered_mask;
    }
};

class TimerEvent : public Event
{
public:
    TimerEvent(const EventSource* source);
};

class ButtonEvent : public GPIOEvent
{
protected:
    uint32_t press_iteration;
public:
    ButtonEvent(const EventSource* source, uint32_t events_triggered_mask, uint32_t press_iteration = 1);

    inline uint32_t GetPressIteration() const
    {
        return press_iteration;
    }

    bool WasPressed() const;
};

class EventSource
{
private:
    void Dispatch(const Event* event) const;

public:
    typedef void (*CallbackAction)(const Event* event, void* user_data);

protected:
    std::unique_ptr<CallbackAction[]> all_actions;
    size_t action_count;
    void* user_data;
    bool is_enabled;

    virtual void EnableImpl() {};
    virtual void DisableImpl() {};

public:
    EventSource(void* user_data = nullptr);
    virtual ~EventSource() = default;

    void SetActions(CallbackAction* all_actions, size_t action_count);
    inline void SetUserData(void* user_data)
    {
        this->user_data = user_data;
    }

    void Enable();
    void Disable();
    inline bool IsEnabled() const
    {
        return is_enabled;
    }

    friend Event;
};

class IRQSource : public EventSource
{
protected:
    virtual void HandleIRQ(uint32_t events_triggered_mask) = 0;

public:
    inline IRQSource(void* user_data = nullptr) : EventSource(user_data) {}
    virtual ~IRQSource() = default;
};