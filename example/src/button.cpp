#include <stdio.h>

#include <hardware/Button.h>

void button1_cb(const Event*, void*)
{
    printf("Button 1 Pressed\n");
}

void button2_cb(const Event*, void*)
{
    printf("Button 2 Pressed\n");
}

int main()
{
    stdio_init_all();

    Button button1 = 16;
    Button button2 = 17;

    int id0 = button1.AddAction(&button1_cb);
    int id1 = button2.AddAction(&button2_cb);

    while (1)
    {
        Event::HandleEvents();
    }

}