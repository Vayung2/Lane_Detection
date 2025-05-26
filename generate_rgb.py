import cv2
import numpy as np

# === CONFIG ===
image_path = "input.jpg"         # Input image path
output_path = "input.rgb"       # Raw RGB output file
width, height = 1280, 720       # Must match the size expected by your FPGA kernel

# === Load and resize image ===
img = cv2.imread(image_path)
if img is None:
    raise FileNotFoundError(f"Cannot load image: {image_path}")

img = cv2.resize(img, (width, height))

# === Convert to raw RGB and save ===
img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)  # Optional: RGB vs BGR
img_rgb.tofile(output_path)

print(f"Saved {width}x{height} RGB image as {output_path}")