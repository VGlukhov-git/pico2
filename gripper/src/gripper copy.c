#define PICO_STDIO_USB_RX_BUFFER_SIZE 1024
#define PICO_STDIO_USB_TX_BUFFER_SIZE 1024

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "modules/vl6180x.h"
#include "modules/adc_reader.h"
#include "modules/led.h"
#include "modules/servo_pwm.h"

#define VL6180X_I2C i2c0
#define VL6180X_SDA_PIN 0
#define VL6180X_SCL_PIN 1
#define VL6180X_BAUDRATE 200000

#define FSR_WRN_LED_PIN 21
#define FSR_GOOD_LED_PIN 20
#define STATUS_LED_PIN 25

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define UART_TX_PIN 16
#define UART_RX_PIN 17
#define UART_BAUDRATE 115200 * 2

#define SERVO1_PIN 18
#define SERVO2_PIN 19

#define PWM_FREQ_HZ 50.0f
#define MIN_PULSE_MS 0.5f
#define MAX_PULSE_MS 2.5f
#define WRAP 65535
#define BUFFER_SIZE 64

typedef struct
{
    uint pin;
    uint slice;
    uint channel;
} Servo;

static Servo servos[2];

static uint16_t angle_to_duty(float angle)
{
    float duty_min = WRAP * (MIN_PULSE_MS / 20.0f);
    float duty_max = WRAP * (MAX_PULSE_MS / 20.0f);
    float duty = duty_min + (angle / 180.0f) * (duty_max - duty_min);

    if (duty < 0.0f)
        duty = 0.0f;
    if (duty > WRAP)
        duty = WRAP;

    return (uint16_t)duty;
}

static void setup_servo(Servo *s, uint pin)
{
    s->pin = pin;
    s->slice = pwm_gpio_to_slice_num(pin);
    s->channel = pwm_gpio_to_channel(pin);

    gpio_set_function(pin, GPIO_FUNC_PWM);

    float div = (float)clock_get_hz(clk_sys) / (PWM_FREQ_HZ * (WRAP + 1));
    pwm_set_clkdiv(s->slice, div);
    pwm_set_wrap(s->slice, WRAP);
    pwm_set_chan_level(s->slice, s->channel, 0);
    pwm_set_enabled(s->slice, true);
}

int main(void)
{
    stdio_init_all();
    stdio_usb_init();
    sleep_ms(2000);

    gpio_init(STATUS_LED_PIN);
    gpio_set_dir(STATUS_LED_PIN, GPIO_OUT);
    gpio_put(STATUS_LED_PIN, 0);

    uart_init(UART_ID, UART_BAUDRATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    sleep_ms(2000);

    i2c_init(VL6180X_I2C, VL6180X_BAUDRATE);
    gpio_set_function(VL6180X_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(VL6180X_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(VL6180X_SDA_PIN);
    gpio_pull_up(VL6180X_SCL_PIN);

    setup_servo(&servos[0], SERVO1_PIN);
    setup_servo(&servos[1], SERVO2_PIN);

    VL6180X sensor;
    ADCReader sensor_adc_left;
    ADCReader sensor_adc_right;
    LED led_warn;
    LED led_good;
    LED led_status;

    uint16_t adc_raw_left;
    uint16_t adc_raw_right;
    uint16_t distance_mm = 0;
    uint8_t status = 0;
    uint8_t raw = 0;

    adc_reader_init(&sensor_adc_left, 26);
    adc_reader_init(&sensor_adc_right, 27);
    led_init(&led_warn, FSR_WRN_LED_PIN);
    led_init(&led_good, FSR_GOOD_LED_PIN);
    led_init(&led_status, STATUS_LED_PIN);

    char buffer[BUFFER_SIZE];
    size_t buffer_len = 0;

    int ret = vl6180x_init(&sensor, VL6180X_I2C, VL6180X_ADDR);

    if (ret != 0)
    {
        printf("VL6180X init failed: %d\n", ret);
        while (1)
        {
            sleep_ms(1000);
        }
    }

    while (true)
    {
        int ch = getchar_timeout_us(0);

        if (ch != PICO_ERROR_TIMEOUT)
        {
            char c = (char)ch;

            if (c == '\n' || c == '\r')
            {
                if (buffer_len > 0)
                {
                    gpio_put(STATUS_LED_PIN, 1);

                    buffer[buffer_len] = '\0';

                    char *sep = strchr(buffer, ';');
                    if (sep != NULL)
                    {
                        *sep = '\0';

                        char *end1;
                        char *end2;

                        long servo_id = strtol(buffer, &end1, 10);
                        float angle = strtof(sep + 1, &end2);

                        if (*end1 == '\0' &&
                            *end2 == '\0' &&
                            servo_id >= 0 &&
                            servo_id <= 1 &&
                            angle >= 0.0f &&
                            angle <= 180.0f)
                        {

                            pwm_set_chan_level(
                                servos[servo_id].slice,
                                servos[servo_id].channel,
                                angle_to_duty(angle));
                        }
                    }

                    buffer_len = 0;
                    gpio_put(STATUS_LED_PIN, 0);
                }
            }
            else
            {
                if (buffer_len < BUFFER_SIZE - 1)
                {
                    buffer[buffer_len++] = c;
                }
                else
                {
                    buffer_len = 0;
                }
            }
        }

        ret = vl6180x_read_range(&sensor, &distance_mm, &status, &raw);

        if (adc_reader_read_raw(&sensor_adc_left, &adc_raw_left) == 0 && adc_reader_read_raw(&sensor_adc_right, &adc_raw_right) == 0)
        {
            if (ret == 0)
            {
                printf("DISTANCE:%u:ADCL:%u:ADCR:%u\n",
                       distance_mm, adc_raw_left, adc_raw_right);
            }
            if (ret != 0)
            {
                printf("DISTANCE:%u:ADCL:%u:ADCR:%u\n",
                       500, adc_raw_left, adc_raw_right);
            }
        }
        sleep_ms(1);
    }

    return 0;
}