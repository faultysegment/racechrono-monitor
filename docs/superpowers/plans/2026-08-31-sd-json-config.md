# MicroSD JSON Configuration & On-Device Edit Mode Elimination Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace on-device parameter editing and hardcoded monitor channels with dynamic MicroSD JSON configuration (`config.json`), global `isHud` mode, auto-creation of default config on empty SD cards, and clean up obsolete edit-mode screens and code.

**Architecture:** `StoragePolicy` handles platform-specific MicroSD / filesystem file I/O (`readConfigFile`, `writeConfigFile`, `isCardPresent`). `AppLogic` uses `ArduinoJson` (v7) on boot to parse `/config.json`, populate dynamic `MonitorConfig` entries and `isHud` in `AppState`, and auto-generate the default JSON template if the file is absent on an inserted card. Screen carousel registrations are simplified by eliminating edit-mode screens and duplicate HUD wrapper screens.

**Tech Stack:** C++11, PlatformIO, ArduinoJson v7, FreeRTOS, TFT_eSPI, Arduino_GFX, SDL2, Unity.

**Spec:** [docs/superpowers/specs/2026-08-31-sd-json-config-design.md](file:///c:/Users/faultysegment/Documents/PlatformIO/Projects/racechrono-monitor/docs/superpowers/specs/2026-08-31-sd-json-config-design.md)

## Global Constraints
- Core classes (`AppLogic.h`, `View.h`, `AppState.h`) must never include hardware-specific headers (`<Arduino.h>`, `<TFT_eSPI.h>`, `<SD.h>`, `<SPI.h>`).
- Cross-platform native builds (unit tests and SDL2 simulator) must compile cleanly on Windows, macOS, and Linux without hardcoded host paths.
- Default monitor fallback: 2 monitors (`TIME` delta formula `"channel(device(lap), delta_lap_time)*100.0"` with limit `0.1`, `SPEED` delta formula `"channel(device(calc), delta_speed)*100"` with limit `5.0`), `isHud: false`.

---

### Task 1: Add `ArduinoJson` dependency in `platformio.ini`

**Files:**
- Modify: `platformio.ini`

**Interfaces:**
- Produces: `<ArduinoJson.h>` available across `T_Embed_CC1101`, `T_Display_S3_AMOLED`, `unit_tests`, and `run_simulator` environments.

- [x] **Step 1: Update `platformio.ini` with `ArduinoJson` library dependency**

Add `bblanchon/ArduinoJson@^7.0.4` to `lib_deps` and configure native environments.

```ini
[env:T_Embed_CC1101]
lib_deps = 
    mathertel/RotaryEncoder@^1.5.3
    lewisxhe/XPowersLib
    bblanchon/ArduinoJson@^7.0.4

[env:T_Display_S3_AMOLED]
lib_deps = 
    bblanchon/ArduinoJson@^7.0.4

[env:unit_tests]
lib_deps = 
    bblanchon/ArduinoJson@^7.0.4

[env:run_simulator]
lib_deps = 
    bblanchon/ArduinoJson@^7.0.4
```

- [x] **Step 2: Run test compilation to verify `ArduinoJson` is downloaded and resolvable**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests`
Expected: Passes or compiles with `ArduinoJson` included.

- [x] **Step 3: Commit**

```bash
git add platformio.ini
git commit -m "build: add ArduinoJson v7 dependency to platformio.ini"
```

---

### Task 2: Update `AppState.h` Data Structures

**Files:**
- Modify: `src/AppState.h`
- Test: `test/test_appstate/test_appstate.cpp`

**Interfaces:**
- Consumes: None
- Produces: `struct MonitorConfig`, `state.monitorConfigs`, `state.numMonitorConfigs`, `state.isHud`, removal of `state.isEditMode`.

- [x] **Step 1: Write unit tests in `test/test_appstate/test_appstate.cpp` for `MonitorConfig` and `isHud`**

- [x] **Step 2: Run test to verify it fails**

- [x] **Step 3: Update `src/AppState.h`**

- [x] **Step 4: Run unit tests to verify they pass**

- [x] **Step 5: Commit**

```bash
git add src/AppState.h test/test_appstate/test_appstate.cpp
git commit -m "feat: add MonitorConfig and isHud state, remove isEditMode from AppState"
```

---

### Task 3: Implement `StoragePolicy` Interface & Platform Implementations

**Files:**
- Modify: `src/Device_Mock/Policies/MockStoragePolicy.h`
- Modify: `src/Device_Native/Policies/RealStoragePolicy.h`
- Modify: `src/Device_T_Display_S3_AMOLED/Policies/AmoledStoragePolicy.h`
- Modify: `src/Device_T_Embed_CC1101/Policies/RealStoragePolicy.h`

**Interfaces:**
- Produces: `StoragePolicy::isCardPresent()`, `StoragePolicy::readConfigFile(const char*)`, `StoragePolicy::writeConfigFile(const char*, const char*)`.

- [x] **Step 1: Update `MockStoragePolicy.h`**

- [x] **Step 2: Update `RealStoragePolicy.h` for Native Simulator**

- [x] **Step 3: Update `AmoledStoragePolicy.h` and `T-Embed RealStoragePolicy.h`**

- [x] **Step 4: Run unit tests to verify compilation**

- [x] **Step 5: Commit**

```bash
git add src/Device_Mock/Policies/MockStoragePolicy.h src/Device_Native/Policies/RealStoragePolicy.h src/Device_T_Display_S3_AMOLED/Policies/AmoledStoragePolicy.h src/Device_T_Embed_CC1101/Policies/RealStoragePolicy.h
git commit -m "feat: implement StoragePolicy MicroSD and file I/O operations"
```

---

### Task 4: Implement JSON Config Parsing & Dynamic Monitor Registration in `AppLogic.h`

**Files:**
- Modify: `src/AppLogic.h`
- Test: `test/test_applogic/test_applogic.cpp`

**Interfaces:**
- Consumes: `StoragePolicy`, `ArduinoJson`, `AppState::MonitorConfig`.
- Produces: `AppLogic::loadConfig()`, `AppLogic::saveDefaultConfig()`, dynamic `configureMonitors()`, removed `toggleEditMode()` and `changeValue()`.

- [x] **Step 1: Write unit tests in `test/test_applogic/test_applogic.cpp`**

- [x] **Step 2: Run test to verify it fails**

- [x] **Step 3: Implement JSON loading, fallback, and dynamic monitor registration in `AppLogic.h`**

- [x] **Step 4: Run unit tests to verify they pass**

- [x] **Step 5: Commit**

```bash
git add src/AppLogic.h test/test_applogic/test_applogic.cpp
git commit -m "feat: parse config.json, dynamic monitor configuration, and remove edit mode"
```

---

### Task 5: UI & View Cleanup (Delete Config Screens & Global HUD)

**Files:**
- Delete: `src/Screens/ConfigMonitorScreen.h`
- Delete: `src/Screens/CircularConfigMonitorScreen.h`
- Modify: `src/CircularUI.h`
- Modify: `src/Screens/CircularMonitorScreen.h`
- Modify: `src/Screens/MonitorScreen.h`
- Modify: `src/View.h`
- Modify: `src/Device_Native/Policies/NativeViewPolicy.h`
- Modify: `src/Device_T_Display_S3_AMOLED/Policies/AmoledViewPolicy.h`
- Modify: `src/Device_T_Embed_CC1101/Policies/TEmbedViewPolicy.h`
- Test: `test/test_view/test_view.cpp`

**Interfaces:**
- Consumes: `state.isHud`, dynamic `state.monitors`.
- Produces: Clean UI without edit mode indicators or config screens, single registration per screen, global HUD mode.

- [x] **Step 1: Delete `ConfigMonitorScreen.h` and `CircularConfigMonitorScreen.h`**

- [x] **Step 2: Remove edit-mode drawing in `CircularUI.h`, `CircularMonitorScreen.h`, and `MonitorScreen.h`**

- [x] **Step 3: Update `NativeViewPolicy.h`, `AmoledViewPolicy.h`, `TEmbedViewPolicy.h`, and `View.h`**

- [x] **Step 4: Update `test/test_view/test_view.cpp` and run tests**

- [x] **Step 5: Commit**

```bash
git add -u
git commit -m "refactor: delete config screens, remove edit mode from UI, apply global HUD mode"
```

---

### Task 6: End-to-End Verification & Default `config.json` Template

**Files:**
- Create: `config.json` (root of project for simulator)
- Test: `env:unit_tests` and `env:run_simulator`

- [x] **Step 1: Create default `config.json` in project root**

- [x] **Step 2: Run all unit tests**

- [x] **Step 3: Run simulator build**

- [x] **Step 4: Commit**

```bash
git add config.json docs/superpowers/plans/2026-08-31-sd-json-config.md
git commit -m "docs: add plan and default config.json for simulator"
```
