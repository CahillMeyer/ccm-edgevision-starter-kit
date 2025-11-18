@echo off
REM --------------------------------------------------
REM Build script for Windows (MSVC)
REM Requires: OpenCV installed at C:\opencv
REM Output: build\test_camera_msvc.exe
REM --------------------------------------------------
IF NOT EXIST build mkdir build
cl src\test_camera_windows.cpp /Fo:build\test_camera_windows.obj /I C:\opencv\build\include /Fe:build\test_camera_msvc.exe /link /LIBPATH:C:\opencv\build\x64\vc16\lib opencv_world4120.lib
