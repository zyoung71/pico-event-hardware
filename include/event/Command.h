#pragma once

#include <util/ArrayView.h>
#include "Event.h"

static constexpr size_t max_command_segment_length = 64;
static constexpr size_t max_command_length = max_command_segment_length * 3;

struct Command
{
    char command_prefix[max_command_segment_length];
    char command_name[max_command_segment_length];
    char command_arguments[max_command_segment_length];

    // All commands are terminated by a new-line character.
    char full_command[max_command_length];

    Command(const ArrayView<char>& prefix, const ArrayView<char>& name, const ArrayView<char>& arguments, const ArrayView<char>& full_command);
    Command(const ArrayView<char>& prefix, const ArrayView<char>& name, const ArrayView<char>& arguments);
    Command(const ArrayView<char>& prefix, const ArrayView<char>& name);

    Command(const char* prefix, const char* name, const char* arguments, const char* full_command);
    Command(const char* prefix, const char* name, const char* arguments);
    Command(const char* prefix, const char* name);

    // None
    Command();

    void ArgPrint(const char* format, ...);
    void ArgScan(const char* format, ...) const;

    bool operator==(const Command& other) const;
    bool operator!=(const Command& other) const;

    const char* GetFullCommand() const;

    bool Is(const char* target_command_name) const;
};

class CommandEvent : public Event
{
protected:
    Command command;

public:
    CommandEvent(EventSourceBase* source, Command&& cmd);

    inline const Command& GetCommand() const
    {
        return command;
    }

};

// List of useful commands for bi-directional interactions.
namespace commands
{
    extern const Command command_ready;
}