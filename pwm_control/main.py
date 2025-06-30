import machine

# This baudrate works fine with Raspberry Pi Zero 2 => Raspberry Pico 2 communication
# You can change RX and TX pins as needed
uart = machine.UART(0, baudrate=115000*2, tx=machine.Pin(16), rx=machine.Pin(17))

servo_pins = {}
for i in range(15):
    pin = machine.Pin(i)
    pwm = machine.PWM(pin)
    pwm.freq(50)
    servo_pins[i] = pwm

min_duty = int(65535 * 0.5 / 20) # Values for mg92b
max_duty = int(65535 * 2.4 / 20) # Values for mg92b

def angle_to_duty(angle):
    return int(min_duty + (angle / 180) * (max_duty - min_duty))


buffer = b""
while True:
    if uart.any():
        char = uart.read(1)
        if char:
            if char == b'\n':
                decoded_buffer = buffer.decode().strip()
                try:
                    parts = decoded_buffer.split(";")
                    value = float(parts[1])
                    motor_id = int(parts[0])
                    if 0 <= motor_id < 15 and 0 <= value <= 180:
                        duty = angle_to_duty(value)
                        servo_pins[motor_id].duty_u16(duty)
                except Exception as e:
                    buffer = b"" 
                    continue
                buffer = b"" 
            else:
                buffer += char
