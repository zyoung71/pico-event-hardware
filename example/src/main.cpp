#include <hardware/Button.h>
#include <hardware/Timer.h>

void act0(const Event* ev, void* user_data)
{
    
}
void act1(const Event* ev, void* user_data)
{
    
}

int main()
{
    Button dev0 = 1;
    StickyButton dev1(0, &dev0, 1);
    RepeatingTimer timer;

    EventSource::CallbackAction actions[] = {act0, act1};

    dev1.SetActions(actions, 2);

    return 0;
}