# AI Agents Guidelines

This document contains architectural decisions, coding standards, and testing strategies adopted in this project. **All AI agents working on this codebase must adhere strictly to these rules.**

## 1. Architectural Pattern: Policy-Based MVC

We employ a highly decoupled **Model-View-Controller (MVC)** architecture. However, instead of using classic C++ interfaces (virtual functions) which introduce vtable overhead, we use **Template-based Policy Injection**.

- **Model (`src/Model.h`)**: Pure data container. Knows nothing about hardware, BLE, or UI. 
- **View (`src/View.h`)**: Pure UI rendering logic. Takes `Model&` and uses a `DisplayPolicy` to draw onto the screen. It internally instantiates its display policy to encapsulate drawing operations.
- **Controller (`src/Controller.h`)**: The central brain. It receives hardware inputs (via `HWPolicy`), updates the `Model`, interacts with BLE (via `BLEPolicy`), and dictates when the `View` should update.

> **CRITICAL RULE**: Core MVC classes (`Controller.h`, `View.h`, `Model.h`) **must NEVER include** hardware-specific libraries (e.g., `<TFT_eSPI.h>`, `<Arduino.h>`, `<NimBLEDevice.h>`). Any hardware interaction must be injected via template policies.

## 2. Device Separation & App Initialization

To prevent `#ifdef` spaghetti, implementations are cleanly separated into dedicated folders inside `src/`:

- **`Device_All/`**: Shared interfaces or structures (e.g., `BLEPolicyCallback`).
- **`Device_T_Embed_CC1101/`**: Real ESP32 firmware implementation.
- **`Device_Mock/`**: Pure software mocks utilized purely for native unit testing.
- **`Device_Native/`**: A full PC Simulator using SDL2.

### Single `main.cpp`
We use a **single, unified `src/main.cpp`**. It delegates immediately to a device-specific `App` class.
Each device folder contains an `App.h` file, which defines an `App` class that instantiates the `Model`, `View`, and `Controller` with the correct hardware policies for that specific target.

## 3. Abstract Hardware Terminology

Core logic must remain agnostic to the physical form-factor of the device.
- Use generic terms: `Action Key`, `Power Key`, `Navigation Delta`.
- **DO NOT** use device-specific terms in core code: e.g., avoid `Encoder Button` or `Encoder Turns` in `Controller` logic.

## 4. Testing & Simulators

This project prioritizes high-speed iteration loops without requiring physical hardware flashing.

### Unit Tests (`env:mock`)
- We run unit tests natively on the PC using PlatformIO's `unity` framework.
- Command: `pio test -e mock`
- Tests run instantly because they use `MockHWPolicy`, `MockDisplayPolicy`, etc., from `Device_Mock`.

### Interactive Integration Test / PC Simulator (`env:run_simulator`)
- We maintain a fully interactive SDL2-based PC Simulator.
- It uses the exact same core `Model`, `View`, and `Controller` logic, acting as an interactive integration test.
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
