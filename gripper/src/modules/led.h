#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t gpio_pin;
    bool state;
    bool initialized;
} LED;

int led_init(LED *led, uint8_t gpio_pin);

void led_on(LED *led);

void led_off(LED *led);

void led_toggle(LED *led);

void led_set(LED *led, bool on);

#endif