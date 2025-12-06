#include <event/Event.h>
#include <hardware/Button.h>

#include <cstring>
#include <algorithm>

queue_t Event::event_queue = {};
Event::_InitializeEventQueue Event::_initialize_event_queue = Event::_InitializeEventQueue();

Event::_InitializeEventQueue::_InitializeEventQueue()
{
    queue_init(&event_queue, sizeof(Event*), 32);
}

void Event::HandleEvents()
{
    Event* event;
    if (queue_try_remove(&event_queue, &event))
    {
        event->source->Dispatch(event);
        delete event;
    }
}

Event::Event(const EventSource* source)
    : source(source)
{
}

GPIOEvent::GPIOEvent(const EventSource* source, uint32_t events_triggered_mask)
    : Event(source), events_triggered_mask(events_triggered_mask)
{
}

TimerEvent::TimerEvent(const EventSource* source)
    : Event(source)
{
}

ButtonEvent::ButtonEvent(const EventSource* source, uint32_t events_triggered_mask, uint32_t press_iteration)
    : GPIOEvent(source, events_triggered_mask), press_iteration(press_iteration)    
{
}

bool ButtonEvent::WasPressed() const
{
    const Button* button = (Button*)source;
    return events_triggered_mask & (button->IsWiredToGround() ? GPIO_IRQ_EDGE_FALL : GPIO_IRQ_EDGE_RISE);
}

CommandEvent::CommandEvent(const EventSource* source, Command&& cmd)
    : Event(source), command(cmd)
{
}

USBUpdateEvent::USBUpdateEvent(const EventSource* source, USBUpdateEventType event_type)
    : Event(source), event_type(event_type)
{
}

/**
 * Event implementation end.
 * ---------------------------
 * Source implementation begin. 
*/

int EventSource::assign_id = 0;

bool EventSource::Callback::operator==(const EventSource::Callback& other) const
{
    return action == other.action && user_data == other.user_data;
}

EventSource::EventSource()
    : is_enabled(true)
{
    event_actions.reserve(3);
    id_table.reserve(3);
}

void EventSource::Dispatch(const Event* ev) const
{
    for (auto&& act : event_actions)
    {
        act.action(ev, act.user_data);
    }
}

int EventSource::AddAction(CallbackAction action, void* user_data)
{
    int id = assign_id++;
    event_actions.emplace_back(action, user_data);
    id_table.emplace(id, event_actions.back());
    return id;
}

void EventSource::RemoveAction(int id)
{
    std::erase(event_actions, id_table[id]);
    id_table.erase(id);
}

void EventSource::Enable()
{
    is_enabled = true;
    EnableImpl();
}

void EventSource::Disable()
{
    is_enabled = false;
    DisableImpl();
}