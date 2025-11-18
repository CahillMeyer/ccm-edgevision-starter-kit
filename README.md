# CCM EdgeVision Starter Kit

A modular object detection system for NVIDIA Jetson, using OpenCV and YOLOv5.

## Features

- Real-time video stream via USB/CSI camera
- ONNX-based YOLOv5 inference pipeline
- Bounding box rendering with class labels
- Flask dashboard for live video preview
- FPS benchmarking + configuration zone support
- Jetson-ready Dockerfile and config system

## Use Cases

- Industrial automation (defect detection, sorting)
- Robotics navigation and tracking
- Retail shelf analytics
- Smart sensors and edge AI prototyping

## Building the Camera Test

This project supports building the camera test executable on both Windows and Linux.

### Windows (MSVC)

1. Open **Developer Command Prompt for Visual Studio**
2. Run:

```bash
build_windows.bat
```

Output binary will be placed in the `build/` directory.

### Linux or WSL (g++)

Make sure `opencv4` and `pkg-config` are installed.

```bash
chmod +x build_linux.sh
./build_linux.sh
```

## Notes

- You can also use VSCode tasks: `Build Camera Test (MSVC)` or `Build Camera Test (g++)`
- Output binaries are generated in the `build/` folder
- Compatible with OpenCV 4.x

## Getting Started

Coming soon: full setup guide for Jetson Nano, Orin NX, and Docker deployment.

## License

MIT — free to use, adapt, and extend.
