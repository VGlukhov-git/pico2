#include "servo_pwm.h"

#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

static float clamp_float(float x, float min_val, float max_val)
{
    if (x < min_val)
        return min_val;
    if (x > max_val)
        return max_val;
    return x;
}

int servo_pwm_init(
    ServoPWM *servo,
    uint8_t gpio_pin,
    float pwm_freq_hz,
    float min_pulse_ms,
    float max_pulse_ms)
{
    if (servo == NULL)
        return -1;

    if (pwm_freq_hz <= 0.0f || min_pulse_ms <= 0.0f || max_pulse_ms <= min_pulse_ms)
        return -1;

    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(gpio_pin);
    uint channel = pwm_gpio_to_channel(gpio_pin);

    const uint32_t wrap = 65535;
    float clkdiv = (float)clock_get_hz(clk_sys) / (pwm_freq_hz * (float)(wrap + 1));

    pwm_set_clkdiv(slice, clkdiv);
    pwm_set_wrap(slice, wrap);
    pwm_set_chan_level(slice, channel, 0);
    pwm_set_enabled(slice, true);

    servo->gpio_pin = gpio_pin;
    servo->slice = slice;
    servo->channel = channel;
    servo->wrap = wrap;
    servo->pwm_freq_hz = pwm_freq_hz;
    servo->min_pulse_ms = min_pulse_ms;
    servo->max_pulse_ms = max_pulse_ms;
    servo->initialized = true;

    return 0;
}

uint16_t servo_pwm_angle_to_level(const ServoPWM *servo, float angle_deg)
{
    if (servo == NULL || !servo->initialized)
        return 0;

    angle_deg = clamp_float(angle_deg, 0.0f, 180.0f);

    float period_ms = 1000.0f / servo->pwm_freq_hz;
    float level_min = (float)servo->wrap * (servo->min_pulse_ms / period_ms);
    float level_max = (float)servo->wrap * (servo->max_pulse_ms / period_ms);

    float level = level_min + (angle_deg / 180.0f) * (level_max - level_min);

    if (level < 0.0f)
        level = 0.0f;
    if (level > (float)servo->wrap)
        level = (float)servo->wrap;

    return (uint16_t)level;
}

int servo_pwm_set_angle(ServoPWM *servo, float angle_deg)
{
    if (servo == NULL || !servo->initialized)
        return -1;

    uint16_t level = servo_pwm_angle_to_level(servo, angle_deg);
    pwm_set_chan_level(servo->slice, servo->channel, level);

    return 0;
}