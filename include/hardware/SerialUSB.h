#pragma once

#include <event/Command.h>

#include <tusb.h>

enum SerialStatus
{
    IDLE,
    WORKING_READ,
    WORKING_WRITE
};

enum USBConnectionStatus
{
    DOWN = 0,
    UP = 1
};

class USBUpdateEvent : public Event
{
public:
    enum USBUpdateEventType
    {
        CONNECTED,
        DISCONNECTED,
        SUSPENDED,
        RESUMED
    };

protected:
    USBUpdateEventType event_type;

public:
    USBUpdateEvent(EventSource* source, USBUpdateEventType event_type);

    inline USBUpdateEventType GetConnectionStatus() const
    {
        return event_type;
    }
};

class SerialUSBDetector;

static SerialUSBDetector* serial_usb_detector_instance = nullptr;

// An event source that fires its actions when a certain cause (a string)
// is matched with the incoming serial.
class SerialUSB : public EventSource
{
private:
    const char** causes_of_events; // What the buffer matches to in order to fire events.
    size_t causes_count;

public:
    static SerialStatus serial_status;

    SerialUSB(const char** causes, size_t causes_count);
    SerialUSB(const char* cause);
    ~SerialUSB();

    bool DetectCommandsOverUSB();
    bool SendCommandOverUSB(const Command& cmd);
};

// Simple event source to detect the physical port.
class SerialUSBDetector : public EventSource
{
private:
    USBConnectionStatus usb_stat;    

public:
    SerialUSBDetector();

    USBConnectionStatus GetContinuousStatus() const
    {
        return usb_stat;
    }

    friend void tud_mount_cb(void);
    friend void tud_umount_cb(void);
    friend void tud_suspend_cb(bool remote_wakeup_en);
    friend void tud_resume_cb(void);
};