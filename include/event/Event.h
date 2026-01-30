#pragma once

#include <vector>
#include <unordered_map>

#include <pico/util/queue.h>

class EventSourceBase;

class Event
{
private:
    struct _InitializeEventQueue
    {
        _InitializeEventQueue();
    };
    static _InitializeEventQueue _initialize_event_queue;

protected:
    EventSourceBase* source;

public:
    static queue_t event_queue;

    static void HandleEvents();

    Event(EventSourceBase* source);
};

class EventSourceBase
{
protected:
    virtual void Dispatch(const Event* event) = 0;

    static int assign_id;

public:
    friend Event;
};

template<class TEventType>
    requires std::is_base_of_v<Event, TEventType>
class EventSource : public EventSourceBase
{
private:
    void Dispatch(const Event* event) override
    {
        TEventType* self_event = (TEventType*)event;
        for (auto& c : event_actions)
            c.action(self_event, this, c.user_data);
    };

public:
    typedef void (*CallbackAction)(const TEventType* event, EventSource* self, void* user_data);
    typedef TEventType EventType;

    template<class TEventSource> requires std::is_base_of_v<EventSource<TEventType>, TEventSource>
    inline TEventSource* GetSourceAsType()
    {
        return (TEventSource*)this;
    }

protected:
    struct Callback
    {
        CallbackAction action;
        void* user_data;

        inline bool operator==(const Callback& other) const
        {
            return action == other.action && user_data == other.user_data;
        }
    };

protected:
    std::vector<Callback> event_actions;
    std::unordered_map<int, Callback> id_table;
    bool is_enabled;

    virtual void EnableImpl() {};
    virtual void DisableImpl() {};

public:
    EventSource() : is_enabled(true)
    {
        event_actions.reserve(3);
        id_table.reserve(3);
    }
    virtual ~EventSource() = default;

    [[nodiscard]] virtual int AddAction(CallbackAction action, void* user_data = nullptr)
    {
        int id = assign_id++;
        event_actions.emplace_back(action, user_data);
        id_table.emplace(id, event_actions.back());
        return id;
    }
    void RemoveAction(int id)
    {
        std::erase(event_actions, id_table[id]);
        id_table.erase(id);
    }

    void Enable()
    {
        is_enabled = true;
        EnableImpl();
    }
    void Disable()
    {
        is_enabled = false;
        DisableImpl();
    }
    inline bool IsEnabled() const
    {
        return is_enabled;
    }
};

template<class TEventType> requires std::is_base_of_v<Event, TEventType>
class IRQSource : public EventSource<TEventType>
{
protected:
    virtual void HandleIRQ(uint32_t events_triggered_mask) = 0;

public:
    inline IRQSource() : EventSource<TEventType>() {}
    virtual ~IRQSource() = default;
};