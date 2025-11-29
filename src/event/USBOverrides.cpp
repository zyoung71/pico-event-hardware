#include <hardware/SerialUSB.h>

#include <tusb.h>

void tud_mount_cb(void)
{
    if (serial_usb_detector_instance)
    {
        serial_usb_detector_instance->usb_stat = CONNECTED;
        if (serial_usb_detector_instance->IsEnabled())
        {
            Event* ev = new USBUpdateEvent(serial_usb_detector_instance, CONNECTED);
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}

void tud_umount_cb(void)
{
    if (serial_usb_detector_instance)
    {
        serial_usb_detector_instance->usb_stat = DISCONNECTED;
        if (serial_usb_detector_instance->IsEnabled())
        {
            Event* ev = new USBUpdateEvent(serial_usb_detector_instance, DISCONNECTED);
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}

void tud_suspend_cb(bool remote_wakeup_en)
{
    if (serial_usb_detector_instance)
    {
        serial_usb_detector_instance->usb_stat = SUSPENDED;
        if (serial_usb_detector_instance->IsEnabled())
        {
            Event* ev = new USBUpdateEvent(serial_usb_detector_instance, SUSPENDED);   
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}

void tud_resume_cb(void)
{
    if (serial_usb_detector_instance)
    {
        serial_usb_detector_instance->usb_stat = CONNECTED; // CONNECTED, as RESUMED is only for instantaneous changes.
        if (serial_usb_detector_instance->IsEnabled())
        {
            Event* ev = new USBUpdateEvent(serial_usb_detector_instance, RESUMED);
            queue_try_add(&Event::event_queue, &ev);
        }
    }
}