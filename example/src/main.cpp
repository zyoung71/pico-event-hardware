#include <hardware/Button.h>
#include <hardware/Timer.h>

#include <stdio.h>

void act0(const Event* ev, void* user_data)
{
    printf("Action 0\n");
}
void act1(const Event* ev, void* user_data)
{
    printf("Action 1\n");
}

void time_action(const Event* ev, void* user_data)
{
    int value = *(int*)user_data;
    printf("Timer fired\tData: %i\n", value);
}

int main()
{
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

    EventSource::CallbackAction actions[] = {&act0, &act1};
    EventSource::CallbackAction timer_action[] = {&time_action};

    // Sets actions with press actions being the first set of arguments.
    button0.SetPressAndReleaseActions(actions, 2);
    button1.SetPressAndReleaseActions(actions, 2);
    
    // Sets actions with release actions being the first set of arguments.
    button2.SetReleaseAndPressActions(actions, 2);
    button3.SetReleaseAndPressActions(actions, 2);

    int value = 500;

    // Set the timer's actions and pass the constant into the actions.
    repeat_timer.SetActions(timer_action, 1);
    repeat_timer.SetUserData(&value);
    
    // Set timer to fire every 3 seconds.
    repeat_timer.Start(3000);

    while (1)
    {
        // Handle events if any were queued.
        Event::HandleEvents();
    }

    return 0;
}