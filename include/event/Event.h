#pragma once

#include "Command.h"

#include <vector>
#include <unordered_map>

#include <pico/util/queue.h>

class EventSource;

class Event
{
private:
    struct _InitializeEventQueue
    {
        _InitializeEventQueue();
    };
    static _InitializeEventQueue _initialize_event_queue;

protected:
    const EventSource* source;

public:
    static queue_t event_queue;

    static void HandleEvents();

    Event(const EventSource* source);

    /**
     * @return The downcasted event type. Always check for a nullptr here.
    */
    template<class EventTemplate> requires std::is_base_of_v<Event, EventTemplate>
    EventTemplate* GetEventAsType() const
    {
        if (EventTemplate* ev = (EventTemplate*)this)
            return ev;
        return nullptr;
    }

    inline const EventSource* GetSource() const
    {
        return source;
    }

    template<class EventSourceTemplate> requires std::is_base_of_v<EventSource, EventSourceTemplate>
    EventSourceTemplate* GetSourceAsType() const
    {
        if (EventSourceTemplate* t_source = (EventSourceTemplate*)source)
            return t_source;
        return nullptr;
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

class CommandEvent : public Event
{
protected:
    Command command;

public:
    CommandEvent(const EventSource* source, Command&& cmd);

    inline const Command& GetCommand() const
    {
        return command;
    }

};

class USBUpdateEvent : public Event
{
public:
    enum USBUpdateEventType
    {
        CONNECTED,
        DISCONNECTED,
        SUSPENDED,
        RESUMED
    };

protected:
    USBUpdateEventType event_type;

public:
    USBUpdateEvent(const EventSource* source, USBUpdateEventType event_type);

    inline USBUpdateEventType GetConnectionStatus() const
    {
        return event_type;
    }
};

class EventSource
{
private:
    virtual void Dispatch(const Event* event) const;

    static int assign_id;

public:
    typedef void (*CallbackAction)(const Event* event, void* user_data);

protected:
    struct Callback
    {
        CallbackAction action;
        void* user_data;

        bool operator==(const Callback& other) const;
    };

protected:
    std::vector<Callback> event_actions;
    std::unordered_map<int, Callback> id_table;
    bool is_enabled;

    virtual void EnableImpl() {};
    virtual void DisableImpl() {};

public:
    EventSource();
    virtual ~EventSource() = default;

    [[nodiscard]] virtual int AddAction(CallbackAction action, void* user_data = nullptr);
    void RemoveAction(int id);

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
    inline IRQSource() : EventSource() {}
    virtual ~IRQSource() = default;
};