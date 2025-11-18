cmake_minimum_required(VERSION 3.10)
project(CCMEdgeVision)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(test_camera src/test_camera_windows.cpp)
target_link_libraries(test_camera ${OpenCV_LIBS})