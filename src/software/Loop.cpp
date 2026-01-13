#include <software/Loop.h>

int Loop::AddAction(CallbackAction action, void* user_data)
{
    _IncludeBreak cb = {action, user_data, break_cb, break_data};
    return EventSource::AddAction([](const Event* ev, void* ptr){
        _IncludeBreak* cb = (_IncludeBreak*)ptr;
        cb->action(ev, cb->user_data);
        if (!cb->break_cb(cb->break_data))
        {
            Event* again = new Event(*ev); // must be copied since the original will be deleted when the callback ends.
            queue_try_add(&Event::event_queue, &again);
        }
    }, &cb);
}