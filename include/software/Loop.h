#pragma once

#include <event/Event.h>

class Loop : public EventSource
{
public:
    typedef bool(*BreakCallback)(void*);    

private:
    struct _IncludeBreak
    {
        CallbackAction action;
        void* user_data;
        BreakCallback break_cb;
        void* break_data;
    };

private:
    static int break_assign_id;

protected:
    BreakCallback break_cb;
    void* break_data;

    std::vector<Callback> break_actions;
    std::unordered_map<int, Callback> break_id_table;

public:
    inline Loop(BreakCallback break_cb, void* break_data = nullptr) : break_cb(break_cb), break_data(break_data) {}

    [[nodiscard]] int AddAction(CallbackAction action, void* user_data = nullptr) override;
    [[nodiscard]] int AddBreakAction(CallbackAction action, void* user_data = nullptr);
    void RemoveBreakAction(int id);
};