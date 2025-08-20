@echo off
echo Building Ray Engine...
echo.

g++ -std=c++17 -O3 -m64 -flto -pthread -mwindows -static-libgcc -static-libstdc++ -o ray_engine rayengine.cpp -lgdi32 -luser32

if %ERRORLEVEL% == 0 (
    echo ✅ Build successful! 
    echo Executable: ray_engine.exe
    echo.
    echo Run with: ray_engine.exe
) else (
    echo ❌ Build failed!
    echo Make sure you have g++ with C++17 support installed.
)

pause
