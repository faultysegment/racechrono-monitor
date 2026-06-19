# RaceChrono Monitor

An **AI Assisted** ESP32-based BLE monitor for RaceChrono, built using PlatformIO.

## Overview

This project acts as an external monitor for RaceChrono, fetching and displaying real-time telemetry like Delta Lap Time, Delta Speed, and other metrics over a Bluetooth Low Energy (BLE) connection. 

It features a TFT display, a navigation interface for changing menus and configuring limits, and relies on a clean, decoupled architecture.

## Clean Architecture
The project employs a robust, highly-decoupled architecture using C++ templates (Policy Injection). The core business logic (`Model`, `View`, `Controller`) is completely hardware-agnostic and relies entirely on injected hardware policies.

Because of this architecture, the project has a single unified `src/main.cpp` and splits implementations into specialized device folders:
- **`Device_T_Embed_CC1101`**: Real hardware implementation using `TFT_eSPI`, ESP32 BLE, and GPIO interrupts.
- **`Device_Mock`**: Pure software mocks used exclusively for blazing fast native unit tests.
- **`Device_Native`**: A fully functional PC-based simulator that renders the UI in a desktop window using SDL2.

## How to Build and Run

1. Install [PlatformIO](https://platformio.org/).
2. Build the firmware for the ESP32 hardware:
   ```bash
   pio run -e T_Embed_CC1101
   ```
3. Upload to the board:
   ```bash
   pio run -e T_Embed_CC1101 -t upload
   ```

## Running Unit Tests (Mock)

To run the full suite of native unit tests (testing the core MVC logic without hardware):

```bash
pio test -e mock
```
This builds and runs tests on your host machine instantly using `Unity` and the policies from `Device_Mock`.

## PC Simulator (Native Build)

You can launch a fully working simulator of the device directly on your PC. The simulator uses SDL2 to draw the screens and capture keyboard input.

> [!TIP]
> **Interactive Integration Test**
> Beyond just UI prototyping, the PC simulator acts as a full-fledged integration test. Since it runs the exact same `Controller`, `Model`, and `View` logic as the real firmware, it allows you to test application flows, state transitions, and BLE mocked interactions natively on your host machine without deploying to hardware!

### Key Bindings
- **Spacebar**: Action Key (Select / Enter Edit Mode)
- **Left/Right Arrows**: Navigation (Menu Next/Prev, Change limits)
- **Escape**: Power Button
- **Enter**: Connect/Disconnect BLE (simulates connection and starts streaming dummy telemetry data)

### Setup
Since the simulator relies on `SDL2` and `SDL2_ttf`, you must install these libraries on your host system before compiling.

**Windows (MSYS2)**:
1. Open your MSYS2 terminal and install the UCRT64 SDL2 packages:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_ttf
   ```

**Linux (Debian/Ubuntu)**:
```bash
sudo apt-get install libsdl2-dev libsdl2-ttf-dev pkg-config
```

**macOS (Homebrew)**:
```bash
brew install sdl2 sdl2_ttf pkg-config
```

### Running the Simulator
1. Download a `.ttf` font (e.g., Arial, Roboto) and place it in the root folder of this project, naming it **`font.ttf`**.
2. Compile and launch the simulator:
   ```bash
   pio run -e run_simulator
   ```
*(The simulator will automatically compile and open in a new window, picking up the right flags for your OS via `sdl2_build.py`)*

## AI Assisted Project

> [!NOTE]  
> This project was developed with **AI Assistance**. While the core architecture and testing strategies were driven by the developer, AI tools were utilized to help write code, debug issues, refine test configurations, and assist with code review.
