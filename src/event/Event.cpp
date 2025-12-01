#include <event/Event.h>
#include <hardware/Button.h>

#include <cstring>

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

EventSource::EventSource(void* user_data)
    : event_actions(nullptr), action_count(0), user_data(user_data), is_enabled(true)
{
}

void EventSource::Dispatch(const Event* ev) const
{
    for (size_t i = 0; i < action_count; i++)
    {
        event_actions[i](ev, user_data);
    }
}

void EventSource::SetActions(CallbackAction* all_actions, size_t action_count)
{
    this->event_actions = std::make_unique<CallbackAction[]>(action_count);
    std::copy(all_actions, all_actions + action_count, this->event_actions.get());
    this->action_count = action_count;
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