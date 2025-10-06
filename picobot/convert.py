# use this script to convert animations to raw file
from PIL import Image
import os, struct

INPUT_DIR = "./animations"   # your BMP folders
OUTPUT_DIR = "./animations_raw"
WIDTH, HEIGHT = 160*2, 128*2   # match your display resolution

os.makedirs(OUTPUT_DIR, exist_ok=True)

def convert_to_rgb565(img):
    img = img.convert("RGB").resize((WIDTH, HEIGHT))   # upscale to screen size
    pixels = img.load()
    raw = bytearray()
    for y in range(HEIGHT):
        for x in range(WIDTH):
            r,g,b = pixels[x,y]
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            raw += struct.pack(">H", rgb565)
    return raw

for anim in os.listdir(INPUT_DIR):
    in_folder = os.path.join(INPUT_DIR, anim)
    if os.path.isdir(in_folder):
        out_folder = os.path.join(OUTPUT_DIR, anim)
        os.makedirs(out_folder, exist_ok=True)
        for fname in sorted(os.listdir(in_folder)):
            if fname.endswith(".bmp"):
                img = Image.open(os.path.join(in_folder, fname))
                raw = convert_to_rgb565(img)
                outname = os.path.join(out_folder, fname.replace(".bmp",".raw"))
                with open(outname,"wb") as f:
                    f.write(raw)
                print("Saved", outname)
