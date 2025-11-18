#!/bin/bash
# --------------------------------------------------
# Build script for Linux (g++)
# Requires: OpenCV4 and pkg-config installed
# Output: build/test_camera_gpp
# --------------------------------------------------
mkdir -p build
g++ src/test_camera_windows.cpp -o build/test_camera_gpp -std=c++11 $(pkg-config --cflags --libs opencv4)
