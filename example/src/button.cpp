#include <stdio.h>

#include <hardware/Button.h>
#include <hardware/ResistorLadder.h>

void button1_cb(const ButtonEvent*, auto self, void*)
{
    printf("Button 1 Pressed\n");
}

void button2_cb(const ButtonEvent*, auto self, void*)
{
    printf("Button 2 Pressed\n");
}

int main()
{
    stdio_init_all();

    Button button1 = 16;
    Button button2 = 17;

    int id1 = button1.AddAction(&button1_cb);
    int id2 = button2.AddAction(&button2_cb);

    AnalogDevice abutton1(26, 200);
    AnalogDevice abutton2(26, 600);
    AnalogDevice abutton3(26, 1050);
    AnalogDevice abutton4(26, 1600);
    AnalogDevice abutton5(26, 2200);
    AnalogDevice abutton6(26, 2800);

    ResistorLadder<6> button_ladder(18, Pull::UP, GPIO_IRQ_EDGE_FALL);
    button_ladder[0] = &abutton1;
    button_ladder[1] = &abutton2;
    button_ladder[2] = &abutton3;
    button_ladder[3] = &abutton4;
    button_ladder[4] = &abutton5;
    button_ladder[5] = &abutton6;

    button_ladder.SortDevices();

    int aid1 = abutton1.AddAction([](const AnalogEvent* ev, auto self, void* ptr){
        printf("Analog Button 1 callback.\n");
    });

    int aid2 = abutton2.AddAction([](const AnalogEvent* ev, auto self, void* ptr){
        printf("Analog Button 2 callback.\n");
    });

    while (1)
    {
        Event::HandleEvents();
    }

}