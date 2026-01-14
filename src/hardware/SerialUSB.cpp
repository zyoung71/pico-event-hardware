#include <hardware/SerialUSB.h>
#include <comms/serial_usb.h>

#include <cstring>

USBUpdateEvent::USBUpdateEvent(EventSource* source, USBUpdateEventType event_type)
    : Event(source), event_type(event_type)
{
}

SerialStatus SerialUSB::serial_status = IDLE;

SerialUSB::SerialUSB(const char** causes, size_t causes_count)
    : EventSource(), causes_of_events(new const char*[causes_count]), causes_count(causes_count)
{
    memcpy(causes_of_events, causes, causes_count * sizeof(char*));
    comms_serial_usb_init();
}

SerialUSB::SerialUSB(const char* cause)
    : SerialUSB(new const char*[1], 1)
{
    causes_of_events[0] = cause;
    comms_serial_usb_init();
}

SerialUSB::~SerialUSB()
{
    delete[] causes_of_events;
}

bool SerialUSB::DetectCommandsOverUSB()
{
    if (serial_status == WORKING_READ || serial_status == WORKING_WRITE)
        return false;

    char buff[max_command_length];
    if (comms_serial_try_read_text_line_over_usb(buff, max_command_length, nullptr) != COMMS_OK)
        return false;
    
    // If any prefix is found, push onto queue.
    for (size_t i = 0; i < causes_count; i++)
    {
        const char* cause = causes_of_events[i];
        if (strncmp(cause, buff, strlen(cause)) == 0)
        {
            char cmd_name_buff[max_command_segment_length], cmd_args_buff[max_command_segment_length];
            sscanf(buff + strlen(cause), "%63s \"%63[^\"]\"", cmd_name_buff, cmd_args_buff);

            Event* ev = new CommandEvent(this, Command(cause, cmd_name_buff, cmd_args_buff, buff));
            queue_try_add(&Event::event_queue, &ev);
            return true;
        }
    }
    return false;
}

bool SerialUSB::SendCommandOverUSB(const Command& cmd)
{
    if (serial_status == WORKING_READ || serial_status == WORKING_WRITE)
        return false;

    return comms_serial_try_write_text_line_over_usb(cmd.full_command, max_command_length) > 0;
}

SerialUSBDetector::SerialUSBDetector()
    : EventSource()
{
    if (!serial_usb_detector_instance)
        serial_usb_detector_instance = this;
}