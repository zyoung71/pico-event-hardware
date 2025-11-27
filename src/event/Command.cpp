#include <event/Command.h>

#include <cstring>

Command::Command(const std::string& prefix, const std::string& name, const std::string& args, const char* full_command)
    : command_prefix(prefix), command_name(name), command_arguments(args), full_command(full_command)
{
}

Command::Command(const std::string& prefix, const std::string& name)
    : command_prefix(prefix), command_name(name), full_command(nullptr)
{
    command_arguments.clear();
}

Command::Command()
    : Command("", "")
{
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
    command_arguments = std::string(buff);
    full_command = GetFullCommand().c_str();
}

void Command::ArgScan(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vsscanf(command_arguments.c_str(), format, args);

    va_end(args);
}

// Prefix and arg count are omitted.
bool Command::operator==(const Command& other) const
{
    return command_name == other.command_name && command_arguments == other.command_arguments;
}

bool Command::operator!=(const Command& other) const
{
    return !(*this == other);
}

std::string Command::GetFullCommand() const
{
    static std::string complete;
    complete = command_prefix + ' ' + command_name + ' ' + command_arguments;
    return complete;
}