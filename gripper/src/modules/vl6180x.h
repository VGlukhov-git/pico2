#ifndef VL6180X_H
#define VL6180X_H

#include <stdint.h>
#include <stddef.h>
#include "hardware/i2c.h"

#define VL6180X_ADDR 0x29

typedef struct
{
    i2c_inst_t *i2c;
    uint8_t addr;
    uint8_t scaling;
} VL6180X;

int vl6180x_init(VL6180X *sensor, i2c_inst_t *i2c, uint8_t address);
int vl6180x_set_scaling(VL6180X *sensor, uint8_t scaling);
int vl6180x_read_range(VL6180X *sensor, uint16_t *distance_mm, uint8_t *status, uint8_t *raw);

#endif