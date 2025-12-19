@echo off
echo ========================================
echo     SDL3 Sample - Build and Run
echo ========================================
echo.

cd /d "%~dp0"

echo Configuring project with MinGW Makefiles...
cmake -S . -B build -G "MinGW Makefiles"
if %errorlevel% neq 0 (
    echo.
    echo *** CMake configuration FAILED! ***
    pause
    exit /b %errorlevel%
)

echo.
echo Building project...
cmake --build build --config Release
if %errorlevel% neq 0 (
    echo.
    echo *** Build FAILED! ***
    pause
    exit /b %errorlevel%
)

echo.
echo === Build successful! Running program... ===
echo.

if exist build\sdl-min.exe (
    build\sdl-min.exe
) else if exist build\Release\sdl-min.exe (
    build\Release\sdl-min.exe
) else (
    echo *** Executable not found! ***
)

echo.
echo ========================================
echo Program finished.
pause