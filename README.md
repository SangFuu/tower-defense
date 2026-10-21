# Tower Defense

I created this C project with SAID PAUL. It is a Tower Defense simulation using SDL2.

Defenses are placed automatically and enemies follow a path towards the king's tower. You can watch the game, pause it, save, load or restart it.

![Game preview](docs/apercu.png)

## Play on Windows

Extract the Windows version and open `tower-defense.exe`. Keep the `data` folder and `SDL2.dll` next to the executable.

| Key | Action |
|---|---|
| P or Space | Pause / resume |
| N | New game |
| S / C | Save / load in binary format |
| D / V | Save / load in text format |
| Escape | Quit |

The window title shows the king's health and the game state. Saves are stored in the application folder (`%APPDATA%/SangFuu/TowerDefense` on Windows).

## Build on Linux

```sh
sudo apt-get install build-essential cmake libsdl2-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/tower-defense
```

## Build on Windows

With 64-bit MinGW, CMake and SDL2 for MinGW:

```powershell
cmake -S . -B build -G Ninja -DSDL2_ROOT="C:/tools/SDL2/x86_64-w64-mingw32" -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/tower-defense.exe
```

CMake copies the required resources and DLL next to the program in this configuration.

## Tests

```sh
ctest --test-dir build --output-on-failure -V
```

Tests cover paths, simulations, attacks, saves and SDL display. Incomplete old saves are rejected without replacing the current game. Text format is preferred when sharing a save between platforms.

Original images are kept. SDL2 is an external dependency under the zlib licence.
