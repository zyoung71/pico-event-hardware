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

protected:
    BreakCallback break_cb;
    void* break_data;

public:
    inline Loop(BreakCallback break_cb, void* break_data = nullptr) : break_cb(break_cb), break_data(break_data) {}

    [[nodiscard]] int AddAction(CallbackAction action, void* user_data = nullptr) override; 
};