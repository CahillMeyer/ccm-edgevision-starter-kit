cmake_minimum_required(VERSION 3.10)
project(CCMEdgeVision)

set(CMAKE_CXX_STANDARD 17) # Updated to 17 for modern features
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

# Include the header files
include_directories(include)
include_directories(${OpenCV_INCLUDE_DIRS})

# Define source files
set(SOURCES
    src/main.cpp
    src/camera_input.cpp
    src/detector.cpp
    src/overlay_renderer.cpp
    src/tracker.cpp
)

add_executable(ccm_edgevision ${SOURCES})
target_link_libraries(ccm_edgevision ${OpenCV_LIBS})