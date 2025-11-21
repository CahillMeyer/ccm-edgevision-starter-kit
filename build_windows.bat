@echo off
REM -------------------------------------------------------------------------
REM Build Script for CCM EdgeVision (Professional Edition)
REM -------------------------------------------------------------------------
REM /EHsc   : Standard Exception Handling
REM /W4     : Warning Level 4 (Strict)
REM /wd4127 : Suppress "conditional expression is constant" (OpenCV noise)
REM -------------------------------------------------------------------------

IF NOT EXIST build mkdir build

echo [BUILD] Compiling CCM EdgeVision (Professional Edition)...

REM We explicitly list source files to avoid linking conflicts with old test files
set SOURCES=src\main.cpp src\camera_input.cpp src\config.cpp src\detector.cpp src\overlay_renderer.cpp src\tracker.cpp

cl /EHsc /W4 /wd4127 %SOURCES% /Fo:build\ /I include /I C:\opencv\build\include ^
   /Fe:build\ccm_edgevision.exe ^
   /link /LIBPATH:C:\opencv\build\x64\vc16\lib opencv_world4120.lib

IF %ERRORLEVEL% EQU 0 (
   echo [SUCCESS] Artifact generated at: build\ccm_edgevision.exe
) ELSE (
   echo [FAILURE] Build encountered errors.
)