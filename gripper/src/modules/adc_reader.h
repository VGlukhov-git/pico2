#ifndef ADC_READER_H
#define ADC_READER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    uint8_t gpio_pin;
    uint8_t adc_input;
    bool initialized;
} ADCReader;

int adc_reader_init(ADCReader *reader, uint8_t gpio_pin);

int adc_reader_read_raw(ADCReader *reader, uint16_t *raw_value);

int adc_reader_read_voltage(ADCReader *reader, float vref, float *voltage);

#endif