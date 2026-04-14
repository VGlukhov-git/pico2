#ifndef SERVO_PWM_H
#define SERVO_PWM_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t gpio_pin;
    int slice;
    int channel;
    uint32_t wrap;
    float pwm_freq_hz;
    float min_pulse_ms;
    float max_pulse_ms;
    bool initialized;
} ServoPWM;

int servo_pwm_init(
    ServoPWM *servo,
    uint8_t gpio_pin,
    float pwm_freq_hz,
    float min_pulse_ms,
    float max_pulse_ms);

int servo_pwm_set_angle(ServoPWM *servo, float angle_deg);

uint16_t servo_pwm_angle_to_level(const ServoPWM *servo, float angle_deg);

#endif