#include <hardware/SerialUSB.h>

#include <tusb.h>

void tud_mount_cb(void)
{
    if (serial_usb_detector_instance)
    {
        serial_usb_detector_instance->usb_stat = UP;
        if (serial_usb_detector_instance->IsEnabled())
        {
            Event* ev = new USBUpdateEvent(serial_usb_detector_instance, USBUpdateEvent::CONNECTED);
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}

void tud_umount_cb(void)
{
    if (serial_usb_detector_instance)
    {
        serial_usb_detector_instance->usb_stat = DOWN;
        if (serial_usb_detector_instance->IsEnabled())
        {
            Event* ev = new USBUpdateEvent(serial_usb_detector_instance, USBUpdateEvent::DISCONNECTED);
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}

void tud_suspend_cb(bool remote_wakeup_en)
{
    if (serial_usb_detector_instance)
    {
        serial_usb_detector_instance->usb_stat = DOWN;
        if (serial_usb_detector_instance->IsEnabled())
        {
            Event* ev = new USBUpdateEvent(serial_usb_detector_instance, USBUpdateEvent::SUSPENDED);   
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}

void tud_resume_cb(void)
{
    if (serial_usb_detector_instance)
    {
        serial_usb_detector_instance->usb_stat = UP; // CONNECTED, as RESUMED is only for instantaneous changes.
        if (serial_usb_detector_instance->IsEnabled())
        {
            Event* ev = new USBUpdateEvent(serial_usb_detector_instance, USBUpdateEvent::RESUMED);
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}