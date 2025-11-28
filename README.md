# **CCM EdgeVision – Jetson / Raspberry Pi C++ Vision Starter**
_A high-performance, production-ready C++17 computer-vision starter for NVIDIA Jetson and Raspberry Pi devices_

CCM EdgeVision is a lightweight, real-time C++ vision framework designed for embedded Linux devices that need **deterministic performance**, **low latency**, and **clear pipeline structure**.

Unlike many tutorials that rely on Python notebooks or ad-hoc scripts, this starter provides:

- A **clean CMake/C++17 codebase**
- A **stable image capture loop** (V4L2 or OpenCV)
- A **GPU/CPU-friendly inference structure**
- A simple but production-relevant **“Danger Zone” detection demo**
- Logging, frame timing, and structured components
- Jetson- and Pi-friendly performance (no GUI overhead, minimal allocations)

It’s designed as a **drop-in starting point** for robotics, automation, industrial systems, and custom embedded vision applications.

---

## **Features**

### 🔷 Real-Time Vision Pipeline
- Zero-copy where possible
- Deterministic frame timing
- Clear stages: Capture → Preprocess → Inference → Zone Logic → Output

### 🔷 “Danger Zone” Demo
- Configurable virtual regions
- Alert when person/object enters zone
- Example logs included for debugging and verification

### 🔷 Cross-Platform Embedded Support
- Works on:
  - NVIDIA Jetson (Nano, Orin, Xavier)
  - Raspberry Pi 4/5
  - Generic Linux SBCs

### 🔷 Clean C++17 Architecture
- No framework bloat
- Simple directory layout
- Easy to extend with custom modules (tracking, OCR, segmentation, etc.)

### 🔷 Logging & Debug Output
- Per-frame detections
- Area/zone filtering
- Timing + diagnostics

---

## **Directory Structure**

```
ccm_edgevision/
│
├── src/
│   ├── capture/          # Frame capture (V4L2/OpenCV)
│   ├── inference/        # YOLO / MobileNet / custom detector hooks
│   ├── logic/            # Danger zone logic, filtering, decision making
│   ├── core/             # Utilities, timing, logging
│   └── app/              # Main runtime loop
│
├── models/               # Put your ONNX / TensorRT engines here
├── assets/               # Sample images or config files
├── build/                # Generated build artifacts (ignored)
└── CMakeLists.txt
```

---

## **Quickstart**

### **1. Clone the repository**
```bash
git clone https://github.com/CahillMeyer/ccm-edgevision.git
cd ccm-edgevision
```

### **2. Build**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### **3. Run**
```bash
./edgevision
```

Optional arguments:

```
--camera 0
--width 640
--height 480
--model models/yolov5s.onnx
```

---

## **Danger Zone Demo**

The starter includes a simple example:  
**Alert when a detected object enters a defined region of interest.**

Logs look like:

```
[Frame 5] RawX=140 Blobs: 1
→ Detection inside zone at (x=30, y=20)
```

This demonstrates:

- Region filtering
- Bounding-box math
- Object classification
- Real-time decision logic

Perfect for robotics or automation systems needing spatial triggers.

---

## **Performance Notes**

This starter is optimized for:

- Embedded ARM CPUs
- Jetson GPU acceleration (optional)
- Low memory churn
- Stable frame times
- Headless execution

---

## **Extending the Pipeline**

Add:

- Tracking (SORT, DeepSORT)
- Segmentation
- Motion detection
- Depth estimation
- Custom triggers
- Real-time streaming

The codebase is intentionally small and modular.

---

## **Screenshots**

*(Insert your composite image here when uploading to GitHub.)*

---

## **Roadmap**

- Multi-zone logic configuration
- TensorRT engine support
- Edge-optimized preprocessing kernels
- ONNX Runtime acceleration
- YAML-based pipeline configuration

---

## **About CCM Code**

Specialized in **Embedded Computer Vision**, **C++ Optimization**, and **real-time engineering** for robotics, IoT, and industrial automation.

📩 info@ccmcode.dev  
🌐 https://ccmcode.dev

---

## **License**

MIT
