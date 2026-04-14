#include "led.h"
#include "pico/stdlib.h"

int led_init(LED *led, uint8_t gpio_pin)
{
    if (led == NULL)
        return -1;

    gpio_init(gpio_pin);
    gpio_set_dir(gpio_pin, GPIO_OUT);
    gpio_put(gpio_pin, 0);

    led->gpio_pin = gpio_pin;
    led->state = false;
    led->initialized = true;

    return 0;
}

void led_on(LED *led)
{
    if (!led || !led->initialized)
        return;

    gpio_put(led->gpio_pin, 1);
    led->state = true;
}

void led_off(LED *led)
{
    if (!led || !led->initialized)
        return;

    gpio_put(led->gpio_pin, 0);
    led->state = false;
}

void led_toggle(LED *led)
{
    if (!led || !led->initialized)
        return;

    led->state = !led->state;
    gpio_put(led->gpio_pin, led->state);
}

void led_set(LED *led, bool on)
{
    if (!led || !led->initialized)
        return;

    gpio_put(led->gpio_pin, on);
    led->state = on;
}