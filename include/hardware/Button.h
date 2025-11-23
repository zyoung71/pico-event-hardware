#pragma once

#include "GPIODevice.h"
#include "Timer.h"

class Button : public GPIODevice
{
protected:
    bool gnd_to_pin;

    virtual void HandleIRQ(uint32_t events_triggered_mask) override;

public:
    Button(uint8_t gpio_pin, bool gnd_to_pin = false, void* user_data = nullptr);
    virtual ~Button() = default;

    void SetPressAndReleaseActions(CallbackAction* press_actions, size_t press_count,
        CallbackAction* release_actions = nullptr, size_t release_count = 0);
    
    void SetReleaseAndPressActions(CallbackAction* release_actions, size_t release_count,
        CallbackAction* press_actions = nullptr, size_t press_count = 0);

    inline bool IsWiredToGround() const
    {
        return gnd_to_pin;
    }

    bool IsActivated() const override;
};

class DoublePressButton : public Button
{
private:
    static void timer_action(const Event* ev, void* user_data);

protected:
    std::unique_ptr<CallbackAction[]> double_press_actions;
    size_t double_press_action_count;
    bool double_press_intermediate; // switched on when button is pressed once, awaiting the second

    absolute_time_t press_window_ms;
    CountdownTimer press_window_timer;

    virtual void HandleIRQ(uint32_t events_triggered_mask) override;

public:
    DoublePressButton(uint8_t gpio_pin, absolute_time_t window_ms = 500, bool gnd_to_pin = false, void* user_data = nullptr);
    virtual ~DoublePressButton() = default;

    void SetDoublePressActions(CallbackAction* double_press_actions, size_t count);

    inline void SetPressTimeWindowMs(absolute_time_t window_ms)
    {
        press_window_ms = window_ms;
    }
};

class TriplePressButton : public DoublePressButton
{
private:
    static void timer_action(const Event* ev, void* user_data);

protected:
    std::unique_ptr<CallbackAction[]> triple_press_actions;
    size_t triple_press_action_count;
    bool triple_press_intermediate; // switched on when button is pressed twice, awaiting the third

    virtual void HandleIRQ(uint32_t events_triggered_mask) override;

public:
    TriplePressButton(uint8_t gpio_pin, absolute_time_t window_ms = 500, bool gnd_to_pin = false, void* user_data = nullptr);
    virtual ~TriplePressButton() = default;

    void SetTriplePressActions(CallbackAction* triple_press_actions, size_t count);
};

class StickyButton : public Button
{
protected:
    const GPIODevice* conditional_devices;
    size_t conditional_device_count;

    virtual void HandleIRQ(uint32_t events_triggered_mask) override;

public:
    StickyButton(uint8_t gpio_pin, const GPIODevice* conditional_devices, size_t conditional_device_count, bool gnd_to_pin = false, void* user_data = nullptr);

};