#pragma once

#include <cstdarg>
#include <string>

static constexpr size_t max_command_length = 256;

struct Command
{
    const std::string command_prefix;
    const std::string command_name;

    // The receiver should know what arguments are passed in.
    // It is treated as a string here.
    std::string command_arguments;

    const char* full_command;

    Command(const std::string& prefix, const std::string& cmd_name, const std::string& arguments, const char* full_command);
    Command(const std::string& prefix, const std::string& cmd_name);

    // None
    Command();

    void ArgPrint(const char* format, ...);
    void ArgScan(const char* format, ...);

    bool operator==(const Command& other) const;
    bool operator!=(const Command& other) const;

    std::string GetFullCommand() const;
};