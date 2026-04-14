#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "pico/stdlib.h"
#include "vl6180x.h"

static int vl6180x_write8(VL6180X *sensor, uint16_t reg, uint8_t data)
{
    uint8_t buf[3];
    buf[0] = (uint8_t)(reg >> 8);
    buf[1] = (uint8_t)(reg & 0xFF);
    buf[2] = data;

    int ret = i2c_write_blocking(sensor->i2c, sensor->addr, buf, 3, false);
    return (ret == 3) ? 0 : -1;
}

static int vl6180x_read8(VL6180X *sensor, uint16_t reg, uint8_t *value)
{
    uint8_t regbuf[2];
    regbuf[0] = (uint8_t)(reg >> 8);
    regbuf[1] = (uint8_t)(reg & 0xFF);

    int ret = i2c_write_blocking(sensor->i2c, sensor->addr, regbuf, 2, true);
    if (ret != 2)
    {
        return -1;
    }

    ret = i2c_read_blocking(sensor->i2c, sensor->addr, value, 1, false);
    if (ret != 1)
    {
        return -1;
    }

    return 0;
}

int vl6180x_set_scaling(VL6180X *sensor, uint8_t scaling)
{
    uint8_t scaler;

    switch (scaling)
    {
    case 1:
        scaler = 0xFD;
        break;
    case 2:
        scaler = 0x7F;
        break;
    case 3:
        scaler = 0x54;
        break;
    default:
        return -1;
    }

    sensor->scaling = scaling;

    if (vl6180x_write8(sensor, 0x0096, 0x00) != 0)
        return -1;
    if (vl6180x_write8(sensor, 0x0097, scaler) != 0)
        return -1;

    return 0;
}

int vl6180x_init(VL6180X *sensor, i2c_inst_t *i2c, uint8_t address)
{
    uint8_t model = 0;
    uint8_t fresh = 0;

    sensor->i2c = i2c;
    sensor->addr = address;
    sensor->scaling = 1;

    if (vl6180x_read8(sensor, 0x0000, &model) != 0)
    {
        return -1;
    }

    printf("Model ID: 0x%02X\n", model);
    if (model != 0xB4)
    {
        printf("Unexpected model ID: 0x%02X\n", model);
        return -2;
    }

    if (vl6180x_read8(sensor, 0x0016, &fresh) != 0)
    {
        return -1;
    }
    printf("Fresh out of reset: %u\n", fresh);

    static const struct
    {
        uint16_t reg;
        uint8_t value;
    } init_data[] = {
        {0x0207, 0x01},
        {0x0208, 0x01},
        {0x0096, 0x00},
        {0x0097, 0xFD},
        {0x00E3, 0x00},
        {0x00E4, 0x04},
        {0x00E5, 0x02},
        {0x00E6, 0x01},
        {0x00E7, 0x03},
        {0x00F5, 0x02},
        {0x00D9, 0x05},
        {0x00DB, 0xCE},
        {0x00DC, 0x03},
        {0x00DD, 0xF8},
        {0x009F, 0x00},
        {0x00A3, 0x3C},
        {0x00B7, 0x00},
        {0x00BB, 0x3C},
        {0x00B2, 0x09},
        {0x00CA, 0x09},
        {0x0198, 0x01},
        {0x01B0, 0x17},
        {0x01AD, 0x00},
        {0x00FF, 0x05},
        {0x0100, 0x05},
        {0x0199, 0x05},
        {0x01A6, 0x1B},
        {0x01AC, 0x3E},
        {0x01A7, 0x1F},
        {0x0030, 0x00},
    };

    for (size_t i = 0; i < sizeof(init_data) / sizeof(init_data[0]); i++)
    {
        if (vl6180x_write8(sensor, init_data[i].reg, init_data[i].value) != 0)
        {
            return -1;
        }
    }

    if (vl6180x_write8(sensor, 0x0011, 0x10) != 0)
        return -1;
    if (vl6180x_write8(sensor, 0x010A, 0x30) != 0)
        return -1;
    if (vl6180x_write8(sensor, 0x003F, 0x46) != 0)
        return -1;
    if (vl6180x_write8(sensor, 0x0031, 0xFF) != 0)
        return -1;
    if (vl6180x_write8(sensor, 0x0040, 0x63) != 0)
        return -1;
    if (vl6180x_write8(sensor, 0x002E, 0x01) != 0)
        return -1;
    if (vl6180x_write8(sensor, 0x001B, 0x09) != 0)
        return -1;
    if (vl6180x_write8(sensor, 0x003E, 0x31) != 0)
        return -1;
    if (vl6180x_write8(sensor, 0x0014, 0x24) != 0)
        return -1;
    if (vl6180x_write8(sensor, 0x001C, 63) != 0)
        return -1;

    if (vl6180x_set_scaling(sensor, 3) != 0)
        return -1;

    if (vl6180x_write8(sensor, 0x0016, 0x00) != 0)
        return -1;

    return 0;
}

int vl6180x_read_range(VL6180X *sensor, uint16_t *distance_mm, uint8_t *status, uint8_t *raw)
{
    uint8_t status_reg = 0;
    uint8_t range_status = 0;
    uint8_t range_raw = 0;
    uint32_t timeout_ms = 500;
    uint32_t elapsed = 0;

    if (vl6180x_write8(sensor, 0x0018, 0x01) != 0)
    {
        return -1;
    }

    while (1)
    {
        if (vl6180x_read8(sensor, 0x004F, &status_reg) != 0)
        {
            return -1;
        }

        if (status_reg & 0x04)
        {
            break;
        }

        if (elapsed >= timeout_ms)
        {
            return -2;
        }

        sleep_ms(5);
        elapsed += 5;
    }

    if (vl6180x_read8(sensor, 0x0062, &range_raw) != 0)
        return -1;
    if (vl6180x_read8(sensor, 0x004D, &range_status) != 0)
        return -1;

    range_status >>= 4;

    if (vl6180x_write8(sensor, 0x0015, 0x07) != 0)
        return -1;

    *raw = range_raw;
    *status = range_status;

    if (range_status != 0)
    {
        *distance_mm = 0;
        return 1;
    }

    *distance_mm = (uint16_t)range_raw * sensor->scaling;
    return 0;
}