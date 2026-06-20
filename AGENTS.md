# AI Agents Guidelines

This document contains architectural decisions, coding standards, and testing strategies adopted in this project. **All AI agents working on this codebase must adhere strictly to these rules.**

## 1. Architectural Pattern: Event-Driven Architecture (Actor Model)

We employ a highly decoupled **Event-Driven Architecture** utilizing a central `EventBus` and FreeRTOS tasks. We have completely moved away from MVC and shared mutable state (like a global Model) to eliminate data races and mutex overhead.

- **AppState (`src/AppState.h`)**: Pure UI state container. It is strictly local to the `UITask` (or main execution thread in Native). No other tasks are allowed to touch it.
- **EventBus (`src/Device_All/EventBus.h`)**: The central communication channel. It is a thread-safe queue implemented using `std::queue`, `std::mutex`, and `std::condition_variable`. All hardware and BLE events are pushed here.
- **AppLogic (`src/AppLogic.h`)**: The central event processor. It receives events from the `EventBus` and mutates the `AppState`, then tells the `View` to redraw.
- **View (`src/View.h`)**: Pure UI rendering logic. Takes `AppState&` and uses a `DisplayPolicy` to draw onto the screen.

> **CRITICAL RULE**: Core classes (`AppLogic.h`, `View.h`, `AppState.h`) **must NEVER include** hardware-specific libraries (e.g., `<TFT_eSPI.h>`, `<Arduino.h>`, `<NimBLEDevice.h>`). Any hardware interaction must be injected via template policies or abstract Event payloads.

## 2. Device Separation & App Initialization

To prevent `#ifdef` spaghetti, implementations are cleanly separated into dedicated folders inside `src/`:

- **`Device_All/`**: Shared interfaces or structures (e.g., `EventBus.h`).
- **`Device_T_Embed_CC1101/`**: Real ESP32 firmware implementation.
- **`Device_Mock/`**: Pure software mocks utilized purely for native unit testing.
- **`Device_Native/`**: A full PC Simulator using SDL2.

### Single `main.cpp` per Device
Each device folder contains an `App.h` and `main.cpp`. The `main.cpp` is responsible for setting up the environment. For ESP32, it creates FreeRTOS tasks (`uiTask`, `inputTask`, `logicTask`) which push/pop events from the `EventBus`.

## 3. Abstract Hardware Terminology

Core logic must remain agnostic to the physical form-factor of the device.
- Use generic terms: `Action Key`, `Power Key`, `Navigation Delta`.
- **DO NOT** use device-specific terms in core code: e.g., avoid `Encoder Button` or `Encoder Turns` in Event definitions.

## 4. Testing & Simulators

This project prioritizes high-speed iteration loops without requiring physical hardware flashing.

### Unit Tests (`env:mock`)
- We run unit tests natively on the PC using PlatformIO's `unity` framework.
- Command: `pio test -e mock`
- Tests run instantly because they use `MockHWPolicy`, `MockDisplayPolicy`, etc., from `Device_Mock`.

### Interactive Integration Test / PC Simulator (`env:run_simulator`)
- We maintain a fully interactive SDL2-based PC Simulator.
- It uses the exact same core `AppState`, `View`, and `AppLogic` logic, acting as an interactive integration test.
- Command: `pio run -e run_simulator`
- **Native Build Rules**: 
  - The native build is completely cross-platform.
  - Do not hardcode OS-specific paths (like MSYS2 Windows paths) in `platformio.ini`. 
  - Instead, the `sdl2_build.py` script automatically manages include and library paths for Windows (MSYS2), macOS (Homebrew), and Linux (`pkg-config`).

## 5. UI and Screen Layout
Screens are abstracted via the `IScreen<DisplayPolicy>` interface. 
When creating new UI states:
- Create a new screen class in the relevant device folders.
- Ensure the screen implementation dynamically scales or respects bounds, preventing overlapping text.
- Use `TTF_RenderText_Shaded` in SDL2 mocks or space-padding to prevent overlapping artifact text when refreshing screens (simulating `TFT_eSPI` behavior).
