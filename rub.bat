@echo off
cmake --build build
if %errorlevel% == 0 (
    echo.
    echo === Running the program ===
    build\sdl-min.exe
) else (
    echo Build failed!
)
pause