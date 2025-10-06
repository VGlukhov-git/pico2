# Animation play demo
# upload this file to your pico project

import os
import uasyncio as asyncio
from machine import Pin, SPI

# === SPI & Display Init ===
spi = SPI(1, baudrate=40000000, sck=Pin(10), mosi=Pin(11))
cs  = Pin(9, Pin.OUT)
dc  = Pin(8, Pin.OUT)
rst = Pin(12, Pin.OUT)

def write_cmd(cmd):
    dc.value(0); cs.value(0); spi.write(bytearray([cmd])); cs.value(1)

def write_data(data):
    dc.value(1); cs.value(0); spi.write(data); cs.value(1)

def reset():
    rst.value(0); asyncio.sleep_ms(50); rst.value(1); asyncio.sleep_ms(50)

def init_display():
    reset()
    write_cmd(0x01); asyncio.sleep_ms(150)
    write_cmd(0x11); asyncio.sleep_ms(500)
    write_cmd(0x36); write_data(bytearray([0x60]))
    write_cmd(0x3A); write_data(bytearray([0x55]))
    write_cmd(0x21); write_cmd(0x13)
    write_cmd(0x29); asyncio.sleep_ms(100)

def set_window(x0, y0, x1, y1):
    write_cmd(0x2A); write_data(bytearray([x0>>8, x0&0xFF, x1>>8, x1&0xFF]))
    write_cmd(0x2B); write_data(bytearray([y0>>8, y0&0xFF, y1>>8, y1&0xFF]))
    write_cmd(0x2C)

async def show_raw_stream(path, w=160*2, h=240, chunk=8):
    set_window(0, 0, w-1, h-1)
    with open(path, "rb") as f:
        for _ in range(0, h, chunk):
            data = f.read(w * 2 * chunk)
            write_data(data)
            await asyncio.sleep(0)

# === Animation player ===
ANIM_DIR = "/animations"

async def play_animations():
    folders = sorted([f for f in os.listdir(ANIM_DIR) if not f.startswith('.')])
    while True:
        for folder in folders:
            frame_files = sorted([f for f in os.listdir(ANIM_DIR + "/" + folder) if f.endswith(".raw")])
            for frame in frame_files:
                await show_raw_stream(ANIM_DIR + "/" + folder + "/" + frame)
                await asyncio.sleep(0.05)

# === Main ===
init_display()
asyncio.run(play_animations())
