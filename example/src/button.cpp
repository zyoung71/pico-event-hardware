#include <stdio.h>

#include <hardware/Button.h>
#include <hardware/ResistorLadder.h>

// void button1_cb(const Event*, void*)
// {
//     printf("Button 1 Pressed\n");
// }

// void button2_cb(const Event*, void*)
// {
//     printf("Button 2 Pressed\n");
// }

#define ADD_TO_RLADDER(x) button_ladder[x - 1] = &abutton ## x

int main()
{
    stdio_init_all();
    adc_init();
    adc_select_input(0);

    // Button button1 = 16;
    // Button button2 = 17;

    // int id1 = button1.AddAction(&button1_cb);
    // int id2 = button2.AddAction(&button2_cb);

    AnalogDevice abutton1(26, 1400);
    AnalogDevice abutton2(26, 2000);
    AnalogDevice abutton3(26, 2500);
    AnalogDevice abutton4(26, 2750);
    AnalogDevice abutton5(26, 3100);
    AnalogDevice abutton6(26, 3375);
    AnalogDevice abutton7(26, 3525);
    AnalogDevice abutton8(26, 3800);

    ResistorLadder<8> button_ladder(16, Pull::UP, GPIO_IRQ_EDGE_FALL);

    ADD_TO_RLADDER(1);
    ADD_TO_RLADDER(2);
    ADD_TO_RLADDER(3);
    ADD_TO_RLADDER(4);
    ADD_TO_RLADDER(5);
    ADD_TO_RLADDER(6);
    ADD_TO_RLADDER(7);
    ADD_TO_RLADDER(8);

    button_ladder.SortDevices();

    int aid1 = abutton1.AddAction([](const Event* ev, void* ptr){
        AnalogEvent* event = ev->GetEventAsType<AnalogEvent>();
        printf("Analog Button 1 callback. ADC: %d\n", event->GetADCValue());
    });

    int aid2 = abutton2.AddAction([](const Event* ev, void* ptr){
        AnalogEvent* event = ev->GetEventAsType<AnalogEvent>();
        printf("Analog Button 2 callback. ADC: %d\n", event->GetADCValue());
    });

    int aid3 = abutton3.AddAction([](const Event* ev, void* ptr){
        AnalogEvent* event = ev->GetEventAsType<AnalogEvent>();
        printf("Analog Button 3 callback. ADC: %d\n", event->GetADCValue());
    });

    int aid4 = abutton4.AddAction([](const Event* ev, void* ptr){
        AnalogEvent* event = ev->GetEventAsType<AnalogEvent>();
        printf("Analog Button 4 callback. ADC: %d\n", event->GetADCValue());
    });

    int aid5 = abutton5.AddAction([](const Event* ev, void* ptr){
        AnalogEvent* event = ev->GetEventAsType<AnalogEvent>();
        printf("Analog Button 5 callback. ADC: %d\n", event->GetADCValue());
    });

    int aid6 = abutton6.AddAction([](const Event* ev, void* ptr){
        AnalogEvent* event = ev->GetEventAsType<AnalogEvent>();
        printf("Analog Button 6 callback. ADC: %d\n", event->GetADCValue());
    });

    int aid7 = abutton7.AddAction([](const Event* ev, void* ptr){
        AnalogEvent* event = ev->GetEventAsType<AnalogEvent>();
        printf("Analog Button 7 callback. ADC: %d\n", event->GetADCValue());
    });

    int aid8 = abutton8.AddAction([](const Event* ev, void* ptr){
        AnalogEvent* event = ev->GetEventAsType<AnalogEvent>();
        printf("Analog Button 8 callback. ADC: %d\n", event->GetADCValue());
    });

    int aid8_imm = abutton8.AddImmediateAction([](const Event* ev, void* ptr){
        asm volatile ("nop");
    });

    while (1)
    {
        Event::HandleEvents();
    }

}