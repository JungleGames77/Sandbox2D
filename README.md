# Sandbox2D v0.7.1

A 2D sandbox game built with C++17 and SFML 3.

## Features

- Procedural terrain, trees and ores
- Mining, block placement and tools
- Inventory and crafting
- Named worlds with local save files
- Automatic saving with an on-screen indicator
- Main menu and item tooltips

## Build (Windows, Visual Studio 2022)

Install SFML 3 using vcpkg, then run from a Developer PowerShell for VS 2022:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Debug
.\build\Debug\Sandbox2D.exe
```

Adjust `CMAKE_TOOLCHAIN_FILE` if vcpkg is installed elsewhere.

Saved worlds are stored in `%APPDATA%\Sandbox2D\worlds`.

## Version

v0.7.1
