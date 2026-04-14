import serial
from time import sleep

COM_PORT = "COM8"   # change this
BAUDRATE = 115200*2

ser = serial.Serial(COM_PORT, BAUDRATE, timeout=0.1)
sleep(1)  # gives the port time to settle

current_angle_0 = 90
k = 1
try:
    while True:
        left_angle = current_angle_0
        right_angle = 180 - current_angle_0
        print(left_angle, right_angle)
        if ser.in_waiting > 0:
            try:
                data = ser.readline().decode("utf-8", errors="ignore").strip()
                if data:
                    print(f"RX: {data}")                    
            except Exception as e:
                print("Read error:", e)

        ser.write(f"0;{left_angle}\n".encode())
        ser.write(f"1;{right_angle}\n".encode())
        sleep(0.001)
        current_angle_0 += k
        if current_angle_0 > 90 or current_angle_0 < 0:
            k = -k

except KeyboardInterrupt:
    print("Stopping...")

finally:
    ser.close()