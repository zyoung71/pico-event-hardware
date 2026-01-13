#include <stdio.h>

#include <hardware/Button.h>
#include <hardware/Timer.h>
#include <hardware/SerialUSB.h>
#include <comms/serial_usb.h>
#include <software/Loop.h>

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

void loop_action(const Event* ev, void* user_data)
{
    printf("Loop called.\n");
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

    // A loop-until object that automatically adds itself to the queue again if the condition provided is not met.
    Loop loop = Loop([](void* ptr){
        return true; // will break out after one event call
    });

    // Set two actions for each button.
    // All actions return an integer ID that is used to remove the action if needed.
    // The compiler will warn you if you do not assign an ID.
    int id0 = button0.AddAction(&act0);
    int id1 = button0.AddAction(&act1);
    int id2 = button1.AddAction(&act0);
    int id3 = button1.AddAction(&act1);

    int value = 500;

    // Set the timer's actions and pass the constant into the actions.
    int id4 = repeat_timer.AddAction(&timer_action, &value);  

    // Set timer to fire every 3 seconds.
    repeat_timer.Start(3000);

    // Set actions for the serial reading detector.
    int id5 = serial_usb.AddAction(&serial_action);

    // Set actions for the physical serial connection callbacks.
    int id6 = serial_usb_detector.AddAction(&serial_detector_action);

    // Set actins for looping object.
    int id7 = loop.AddAction(&loop_action);

    // Signal LED to verify setup was completed.
    gpio_put(PICO_DEFAULT_LED_PIN, true);

    constexpr size_t chunk = 32;
    char rx[chunk];
    char tx[chunk];
    size_t bytes_read = 0;
    size_t this_bytes_read = 0;

#ifdef RUN_EXAMPLE_1
    while (1)
    {
        // Handle events if any were queued.
        Event::HandleEvents();
        
        // Check serial port.
        serial_usb.DetectCommandsOverUSB();

        if (SerialUSB::serial_status == WORKING_READ)
        {
            comms_serial_try_read_buff_over_usb_quick(rx, chunk, &this_bytes_read);
            bytes_read += this_bytes_read;
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