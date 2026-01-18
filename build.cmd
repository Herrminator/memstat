@echo off
cmake -S . -B build -G "Ninja Multi-Config"
if errorlevel 1 exit
cmake --build build --config Debug
if errorlevel 1 exit
cmake --build build --config Release
