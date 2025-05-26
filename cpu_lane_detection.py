import cv2
import numpy as np
import subprocess
import csv
import os

# Create log file with header if it doesn't exist
log_file = "cpu_lane_profile_log.csv"
if not os.path.exists(log_file):
    with open(log_file, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=[
            "grayscale", "gaussian_blur", "canny",
            "roi_mask", "hough_transform", "draw_lines", "total"
        ])
        writer.writeheader()

# def grayscale(img):         # CPU Version
#     return cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

def grayscale(img):          # FPGA Version
    # Save current frame as RGB raw input
    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img_rgb.tofile("input.rgb")

    # --- SSH + SCP commands ---
    remote = "vgupta8@hal-fpga-x86.ncsa.illinois.edu"
    remote_dir = "~/grayscale_vitis/data"

    # Send to FPGA
    subprocess.run(["scp", "input.rgb", f"{remote}:{remote_dir}/"], check=True)

    # Run FPGA grayscale
    subprocess.run([
        "ssh", remote,
        "cd ~/grayscale_vitis && scl enable devtoolset-9 'bash -c \""
        "source /opt/xilinx/Vitis/2022.1/settings64.sh && "
        "source /opt/xilinx/xrt/setup.sh && "
        "./host grayscale_kernel.xclbin\"'"
    ], shell=False, check=True)

    # Retrieve result
    subprocess.run(["scp", f"{remote}:{remote_dir}/output.pgm", "."], check=True)

    # Load back into OpenCV as grayscale
    gray = cv2.imread("output.pgm", cv2.IMREAD_GRAYSCALE)
    return gray

def gaussian_blur(img, kernel_size=5):
    return cv2.GaussianBlur(img, (kernel_size, kernel_size), 0)

def canny(img, low_thresh=50, high_thresh=150):
    return cv2.Canny(img, low_thresh, high_thresh)

def region_of_interest(img):
    height = img.shape[0]
    polygons = np.array([[
        (100, height), (img.shape[1]-100, height), (img.shape[1]//2, height//2)
    ]])
    mask = np.zeros_like(img)
    cv2.fillPoly(mask, polygons, 255)
    return cv2.bitwise_and(img, mask)

def draw_lines(img, lines):
    line_img = np.zeros_like(img)
    if lines is None:
        return img
    for line in lines:
        for x1, y1, x2, y2 in line:
            cv2.line(line_img, (x1, y1), (x2, y2), (0, 255, 0), 5)
    return cv2.addWeighted(img, 0.8, line_img, 1, 1)

def hough_lines(img):
    return cv2.HoughLinesP(img, 1, np.pi/180, threshold=50, minLineLength=50, maxLineGap=150)

import time

def process_frame(frame):
    timings = {}
    
    t0 = time.time()
    gray = grayscale(frame)
    timings["grayscale"] = time.time() - t0

    t0 = time.time()
    blur = gaussian_blur(gray)
    timings["gaussian_blur"] = time.time() - t0

    t0 = time.time()
    edges = canny(blur)
    timings["canny"] = time.time() - t0

    t0 = time.time()
    roi = region_of_interest(edges)
    timings["roi_mask"] = time.time() - t0

    t0 = time.time()
    lines = hough_lines(roi)
    timings["hough_transform"] = time.time() - t0

    t0 = time.time()
    output = draw_lines(frame, lines)
    timings["draw_lines"] = time.time() - t0

    timings["total"] = sum(timings.values())

    # Append timing data to CSV log file
    with open(log_file, "a", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=timings.keys())
        writer.writerow(timings)

    return output

# For video input
cap = cv2.VideoCapture("road.mp4")  # Use 0 for webcam or path to video/image

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break
    result = process_frame(frame)
    cv2.imshow("Lane Detection", result)
    if cv2.waitKey(1) == 27:  # ESC to quit
        break

cap.release()
cv2.destroyAllWindows()