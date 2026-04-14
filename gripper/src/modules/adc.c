#include "adc_reader.h"

#include "pico/stdlib.h"
#include "hardware/adc.h"

static bool adc_hw_initialized = false;

static int adc_gpio_to_input(uint8_t gpio_pin)
{
    switch (gpio_pin)
    {
    case 26:
        return 0;
    case 27:
        return 1;
    case 28:
        return 2;
    case 29:
        return 3;
    default:
        return -1;
    }
}

int adc_reader_init(ADCReader *reader, uint8_t gpio_pin)
{
    if (reader == NULL)
    {
        return -1;
    }

    int adc_input = adc_gpio_to_input(gpio_pin);
    if (adc_input < 0)
    {
        return -1;
    }

    if (!adc_hw_initialized)
    {
        adc_init();
        adc_hw_initialized = true;
    }

    adc_gpio_init(gpio_pin);

    reader->gpio_pin = gpio_pin;
    reader->adc_input = (uint8_t)adc_input;
    reader->initialized = true;

    return 0;
}

int adc_reader_read_raw(ADCReader *reader, uint16_t *raw_value)
{
    if (reader == NULL || raw_value == NULL || !reader->initialized)
    {
        return -1;
    }

    adc_select_input(reader->adc_input);
    *raw_value = adc_read();

    return 0;
}

int adc_reader_read_voltage(ADCReader *reader, float vref, float *voltage)
{
    if (reader == NULL || voltage == NULL || !reader->initialized)
    {
        return -1;
    }

    uint16_t raw = 0;
    if (adc_reader_read_raw(reader, &raw) != 0)
    {
        return -1;
    }

    *voltage = ((float)raw * vref) / 4095.0f;
    return 0;
}