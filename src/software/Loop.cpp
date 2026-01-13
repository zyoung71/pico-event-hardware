#include <software/Loop.h>

int Loop::break_assign_id = 0;

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
            return;
        }
        else
        {
            Loop* this_ref = ev->GetSourceAsType<Loop>();
            for (auto&& act : this_ref->break_actions)
            {
                act.action(ev, act.user_data);
            }
        }
    }, &cb);
}

int Loop::AddBreakAction(CallbackAction action, void* user_data)
{
    int id = break_assign_id++;
    break_actions.emplace_back(action, user_data);
    break_id_table.emplace(id, break_actions.back());
    return id;  
}

void Loop::RemoveBreakAction(int id)
{
    std::erase(break_actions, break_id_table[id]);
    id_table.erase(id);
}