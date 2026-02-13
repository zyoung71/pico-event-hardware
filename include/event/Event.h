#pragma once

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
    EventSource* source;

public:
    static queue_t event_queue;

    static void HandleEvents();

    Event(EventSource* source);

    template<class EventTemplate> requires std::is_base_of_v<Event, EventTemplate>
    EventTemplate* GetEventAsType() const
    {
        return (EventTemplate*)this;
    }

    inline EventSource* GetSource() const
    {
        return source;
    }

    template<class EventSourceTemplate> requires std::is_base_of_v<EventSource, EventSourceTemplate>
    EventSourceTemplate* GetSourceAsType() const
    {
        return (EventSourceTemplate*)source;
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
    std::vector<Callback> event_actions_immediate;
    std::unordered_map<int, Callback> id_table;
    bool is_enabled;

    virtual void EnableImpl() {};
    virtual void DisableImpl() {};

    void ProcessImmediateActions(const Event* ev) const;

public:
    EventSource();
    virtual ~EventSource() = default;

    [[nodiscard]] virtual int AddAction(CallbackAction action, void* user_data = nullptr);
    [[nodiscard]] virtual int AddImmediateAction(CallbackAction action, void* user_data = nullptr);
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