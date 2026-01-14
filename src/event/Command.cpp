#include <event/Command.h>

#include <cstring>
#include <stdio.h>
#include <cstdarg>

CommandEvent::CommandEvent(EventSource* source, Command&& cmd)
    : Event(source), command(cmd)
{
}

Command::Command(const ArrayView<char>& prefix, const ArrayView<char>& name, const ArrayView<char>& args, const ArrayView<char>& full)
{
    strncpy(command_prefix, prefix.data, prefix.length);
    strncpy(command_name, name.data, name.length);
    strncpy(command_arguments, args.data, args.length);
    strncpy(full_command, full.data, full.length);
}

Command::Command(const ArrayView<char>& prefix, const ArrayView<char>& name, const ArrayView<char>& args)
{
    strncpy(command_prefix, prefix.data, prefix.length);
    strncpy(command_name, name.data, name.length);
    strncpy(command_arguments, args.data, args.length);
    snprintf(full_command, max_command_length, "%s %s %s", command_prefix, command_name, command_arguments);
    strncat(full_command, "\n", max_command_length);
}

Command::Command(const ArrayView<char>& prefix, const ArrayView<char>& name)
{
    strncpy(command_prefix, prefix.data, prefix.length);
    strncpy(command_name, name.data, name.length);
    memset(command_arguments, 0, max_command_segment_length);
    snprintf(full_command, max_command_length, "%s %s", command_prefix, command_name);
    strncat(full_command, "\n", max_command_length);
}

Command::Command(const char* prefix, const char* name, const char* args, const char* full)
{
    strncpy(command_prefix, prefix, max_command_segment_length);
    strncpy(command_name, name, max_command_segment_length);
    strncpy(command_arguments, args, max_command_segment_length);
    strncpy(full_command, full, max_command_length);
}

Command::Command(const char* prefix, const char* name, const char* args)
{
    strncpy(command_prefix, prefix, max_command_segment_length);
    strncpy(command_name, name, max_command_segment_length);
    strncpy(command_arguments, args, max_command_segment_length);
    snprintf(full_command, max_command_length, "%s%s \"%s\"", command_prefix, command_name, command_arguments);
    strncat(full_command, "\n", max_command_length);
}

Command::Command(const char* prefix, const char* name)
{
    strncpy(command_prefix, prefix, max_command_segment_length);
    strncpy(command_name, name, max_command_segment_length);
    memset(command_arguments, 0, max_command_segment_length);
    snprintf(full_command, max_command_length, "%s%s", command_prefix, command_name);
    strncat(full_command, "\n", max_command_length);
}

Command::Command()
{
    memset(command_prefix, 0, max_command_segment_length);
    memset(command_name, 0, max_command_segment_length);
    memset(command_arguments, 0, max_command_segment_length);
    memset(full_command, 0, max_command_length);
}

void Command::ArgPrint(const char* format, ...)
{
    size_t len = strlen(format) + 1;
    char buff[len];

    va_list args;
    va_start(args, format);

    vsnprintf(buff, len, format, args);

    va_end(args);

    buff[len - 1] = '\0';
    strncpy(command_arguments, buff, len);
    // No space between first two to allow prefixes such as "/command" or "-command"
    snprintf(full_command, max_command_length, "%s%s \"%s\"", command_name, command_prefix, command_arguments);
    strncat(full_command, "\n", max_command_length);
}

void Command::ArgScan(const char* format, ...) const
{
    va_list args;
    va_start(args, format);

    vsscanf(command_arguments, format, args);
    
    va_end(args);
}

bool Command::operator==(const Command& other) const
{
    return strncmp(full_command, other.full_command, max_command_length) == 0;
}

bool Command::operator!=(const Command& other) const
{
    return !(*this == other);
}

const char* Command::GetFullCommand() const
{
    return full_command;
}

bool Command::Is(const char* target_command_name) const
{
    return strncmp(command_name, target_command_name, max_command_segment_length) == 0;
}