#!/bin/bash
# -------------------------------------------------------------------------
# Build Script for CCM EdgeVision (Linux/WSL)
# -------------------------------------------------------------------------
# -std=c++17 : Required for modern C++ features
# -I include : Include the header directory
# -------------------------------------------------------------------------

# 1. Create build directory
mkdir -p build

echo "[BUILD] Compiling CCM EdgeVision (Professional Edition)..."

# 2. Define Source Files (Explicit list to avoid linking old test files)
SOURCES="src/main.cpp src/camera_input.cpp src/config.cpp src/detector.cpp src/overlay_renderer.cpp src/tracker.cpp"

# 3. Compile
# We use pkg-config to automatically find OpenCV paths
g++ $SOURCES -o build/ccm_edgevision \
    -std=c++17 \
    -I include \
    $(pkg-config --cflags --libs opencv4)

# 4. Check Status
if [ $? -eq 0 ]; then
    echo "✅ [SUCCESS] Artifact generated at: build/ccm_edgevision"
    echo "👉 Run with: ./build/ccm_edgevision --test"
else
    echo "❌ [FAILURE] Build encountered errors."
fi