#include <hardware/PulseWidthModulation.h>

#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>

PulseWidthModulation::PulseWidthModulation(uint8_t gpio_pin, float frequency, float duty_cycle)
    : PulseWidthModulation(gpio_pin)
{
    SetFrequency(frequency);
    SetDutyCycle(duty_cycle);
}

PulseWidthModulation::PulseWidthModulation(uint8_t gpio_pin) : gpio_pin(gpio_pin)
{
    slice = pwm_gpio_to_slice_num(gpio_pin);
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
}

void PulseWidthModulation::SetFrequency(float frequency)
{
    
    clock = (float)clock_get_hz(clk_sys);
    float divider = clock / (frequency * 65536.f);

    if (divider < 1.f)
        divider = 1.f;
    if (divider > 255.f)
        divider = 255.f;

    wrap = (uint16_t)(clock / (divider * frequency)) - 1;

    pwm_set_clkdiv(slice, divider);
    pwm_set_wrap(slice, wrap);

    uint channel = pwm_gpio_to_channel(gpio_pin);
    pwm_set_chan_level(slice, channel, 0);
}

void PulseWidthModulation::SetDutyCycle(float duty_cycle)
{
    if (duty_cycle < 0.f)
        duty_cycle = 0.f;
    if (duty_cycle > 1.f)
        duty_cycle = 1.f;

    pwm_set_chan_level(slice, channel, duty_cycle * (wrap + 1));
}

void PulseWidthModulation::SetEnabled(bool enable)
{
    pwm_set_enabled(slice, enable);
}