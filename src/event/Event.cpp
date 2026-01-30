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

Event::Event(EventSourceBase* source)
    : source(source)
{
}

/**
 * Event implementation end.
*/

int EventSourceBase::assign_id = 0;