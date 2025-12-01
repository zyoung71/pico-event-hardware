#include <stdio.h>

#include <hardware/Button.h>
#include <hardware/Timer.h>
#include <hardware/SerialUSB.h>
#include <comms/serial_usb.h>

void act0(const Event* ev, void* user_data)
{
    printf("Action 0\n");
}
void act1(const Event* ev, void* user_data)
{
    printf("Action 1\n");
}

void timer_action(const Event* ev, void* user_data)
{
    int value = *(int*)user_data;
    printf("Timer fired\tData: %i\n", value);
}

void serial_action(const Event* ev, void* user_data)
{
    auto real_event = ev->GetEventAsType<CommandEvent>();
    auto source = ev->GetSourceAsType<SerialUSB>();
    if (real_event && source)
    {
        const Command& cmd = real_event->GetCommand();
        printf("Serial: %s\n", cmd.GetFullCommand());
        
        // Example: uploading a file. In this case, it is printed to stdout.
        if (strncmp(cmd.command_name, "uploadfile", max_command_segment_length) == 0)
        {
            constexpr size_t chunk = 32;

            size_t file_size;
            char file_name[max_command_segment_length];
            cmd.ArgScan("%u \"%s\"", &file_size, file_name);

            source->SendCommandOverUSB(Command("__cmd__", "ready")); // Ready for data to be sent.
            SerialUSB::serial_status = WORKING_READ;

            size_t bytes_written;
            char buff[chunk];
            comms_serial_try_read_buff_over_usb_quick(buff, chunk);
        }
        
    }
}

void serial_detector_action(const Event* ev, void* user_data)
{
    auto real_event = ev->GetEventAsType<USBUpdateEvent>();
    if (real_event)
    {
        printf("Serial Connection Event: %d", real_event->GetConnectionStatus());
    }
}

#define RUN_EXAMPLE_1

int main()
{
    // Note that when reading prints from the Pico, it is sent as serial data and is crossed
    // between the CDC too. If using bi-directional communication, serial prints must be turned off.
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
    // Standard button.
    Button button0 = 0;
    
    // Button that requires two presses within a time window to fire its actions. Default time window of 500ms.
    DoublePressButton button1 = 1;

    // Button that requires three presses within a time window to fire its actions. Default time window of 500ms.
    TriplePressButton button2 = 2;

    // Button that requires button0 to be pressed to fire its actions. It can support more than one requirement.
    // Abstracted example: Ctrl + Shift + This Button
    StickyButton button3 = StickyButton(3, &button0, 1);

    // Timer that repeats every given time frame until it is manually ended or deleted.
    RepeatingTimer repeat_timer;

    // The USB port reading serial data. Max buffer size is 256.
    // This will fire the linked actions if the given string is read from the serial port.
    SerialUSB serial_usb = SerialUSB("__cmd__");

    // Physical USB detection.
    SerialUSBDetector serial_usb_detector;

    EventSource::CallbackAction actions[] = {&act0, &act1};
    EventSource::CallbackAction timer_action_arr[] = {&timer_action};
    EventSource::CallbackAction serial_action_arr[] = {&serial_action};
    EventSource::CallbackAction serial_detector_action_arr[] = {&serial_detector_action};

    // Sets actions with press actions being the first set of arguments.
    button0.SetPressAndReleaseActions(actions, 2);
    button1.SetPressAndReleaseActions(actions, 2);
    
    // Sets actions with release actions being the first set of arguments.
    button2.SetReleaseAndPressActions(actions, 2);
    button3.SetReleaseAndPressActions(actions, 2);

    int value = 500;

    // Set the timer's actions and pass the constant into the actions.
    repeat_timer.SetActions(timer_action_arr, 1);
    repeat_timer.SetUserData(&value);

    // Set timer to fire every 3 seconds.
    repeat_timer.Start(3000);

    // Set actions for the serial reading detector.
    serial_usb.SetActions(serial_action_arr, 1);

    // Set actions for the physical serial connection callbacks.
    serial_usb_detector.SetActions(serial_detector_action_arr, 1);

    // Signal LED to verify setup was completed.
    gpio_put(PICO_DEFAULT_LED_PIN, true);

    constexpr size_t chunk = 32;
    char rx[chunk];
    char tx[chunk];

#ifdef RUN_EXAMPLE_1
    while (1)
    {
        // Handle events if any were queued.
        Event::HandleEvents();
        
        // Check serial port.
        serial_usb.DetectCommandsOverUSB();


        if (SerialUSB::serial_status == WORKING_READ)
        {
            comms_serial_try_read_buff_over_usb_quick(rx, chunk);
            rx[chunk - 1] = '\0'; // Obviously do not do this for data transfers.
            printf("Read buffer chunk: %s\n", rx);
        }
        else if (SerialUSB::serial_status == WORKING_WRITE)
        {
            comms_serial_try_write_buff_over_usb_quick(tx, chunk);
        }
    }
#endif
    return 0;
}