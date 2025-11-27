#pragma once

#include <event/Event.h>
#include <event/Command.h>

enum SerialStatus
{
    IDLE,
    WORKING
};

class SerialUSBDetector;

static SerialUSBDetector* serial_usb_detector_instance = nullptr;

// An event source that fires its actions when a certain cause (a string)
// is matched with the incoming serial.
class SerialUSB : public EventSource
{
protected:
    const char** causes_of_events; // What the buffer matches to in order to fire events.
    size_t causes_count;

public:
    static SerialStatus serial_status;

    SerialUSB(const char** causes_of_events, size_t causes_count, void* user_data = nullptr);
    SerialUSB(const char* cause_of_events, void* user_data = nullptr);

    bool DetectCommandsOverUSB();
    bool SendCommandOverUSB(const Command& cmd);
};

// Simple event source to detect the physical port.
class SerialUSBDetector : public EventSource
{
public:
    SerialUSBDetector(void* user_data = nullptr);
};