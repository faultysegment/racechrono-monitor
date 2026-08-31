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

- [ ] **Step 1: Update `platformio.ini` with `ArduinoJson` library dependency**

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

- [ ] **Step 2: Run test compilation to verify `ArduinoJson` is downloaded and resolvable**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests`
Expected: Passes or compiles with `ArduinoJson` included.

- [ ] **Step 3: Commit**

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

- [ ] **Step 1: Write unit tests in `test/test_appstate/test_appstate.cpp` for `MonitorConfig` and `isHud`**

```cpp
void test_appstate_monitor_configs(void) {
    AppState s;
    s.reset();
    TEST_ASSERT_EQUAL(0, s.numMonitorConfigs);
    TEST_ASSERT_FALSE(s.isHud);

    s.isHud = true;
    TEST_ASSERT_TRUE(s.isHud);

    MonitorConfig cfg;
    strncpy(cfg.name, "Delta time", sizeof(cfg.name));
    strncpy(cfg.title, "TIME", sizeof(cfg.title));
    strncpy(cfg.formula, "channel(device(lap), delta_lap_time)*100.0", sizeof(cfg.formula));
    cfg.multiplier = 0.01f;
    cfg.positiveIsGood = false;
    cfg.decimals = 2;
    cfg.limit = 0.1f;

    s.addMonitorConfig(cfg);
    TEST_ASSERT_EQUAL(1, s.numMonitorConfigs);
    TEST_ASSERT_EQUAL_STRING("TIME", s.monitorConfigs[0].title);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, s.monitorConfigs[0].limit);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests -f test_appstate`
Expected: FAIL (missing `MonitorConfig`, `addMonitorConfig`, `isHud`).

- [ ] **Step 3: Update `src/AppState.h`**

Define `MonitorConfig`, remove `isEditMode`, add `monitorConfigs`, `numMonitorConfigs`, `isHud`, `addMonitorConfig(const MonitorConfig& cfg)` and `clearMonitorConfigs()`.

```cpp
struct MonitorConfig {
    char name[MONITOR_NAME_MAX + 1];
    char title[16];
    char formula[128];
    float multiplier;
    bool positiveIsGood;
    int decimals;
    float limit;
};

// In AppState:
bool isHud = false;
MonitorConfig monitorConfigs[MAX_MONITORS];
int numMonitorConfigs = 0;

void clearMonitorConfigs() {
    numMonitorConfigs = 0;
}

bool addMonitorConfig(const MonitorConfig& cfg) {
    if (numMonitorConfigs < MAX_MONITORS) {
        monitorConfigs[numMonitorConfigs++] = cfg;
        return true;
    }
    return false;
}
```

- [ ] **Step 4: Run unit tests to verify they pass**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests -f test_appstate`
Expected: PASS.

- [ ] **Step 5: Commit**

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

- [ ] **Step 1: Update `MockStoragePolicy.h`**

```cpp
#pragma once
#include <string>
#include <map>

class MockStoragePolicy {
public:
    static std::map<std::string, float> store;
    static std::map<std::string, int> storeInt;
    static bool cardPresent;
    static std::string configFileContent;
    static std::string lastWrittenFileContent;

    static void init() {}
    
    static bool isCardPresent() {
        return cardPresent;
    }

    static std::string readConfigFile(const char* filename = "/config.json") {
        return configFileContent;
    }

    static bool writeConfigFile(const char* filename, const char* content) {
        lastWrittenFileContent = content;
        configFileContent = content;
        return true;
    }

    static float getFloat(const char* key, float defaultValue) {
        if (store.find(key) != store.end()) return store[key];
        return defaultValue;
    }
    
    static void putFloat(const char* key, float value) {
        store[key] = value;
    }

    static int getInt(const char* key, int defaultValue) {
        if (storeInt.find(key) != storeInt.end()) return storeInt[key];
        return defaultValue;
    }

    static void putInt(const char* key, int value) {
        storeInt[key] = value;
    }

    static void reset() {
        store.clear();
        storeInt.clear();
        cardPresent = true;
        configFileContent = "";
        lastWrittenFileContent = "";
    }
};
```

- [ ] **Step 2: Update `RealStoragePolicy.h` for Native Simulator**

```cpp
#pragma once
#include <map>
#include <string>
#include <fstream>
#include <sstream>

class RealStoragePolicy {
    static std::map<std::string, float> store;
    static std::map<std::string, int> storeInt;

public:
    static void init() {}
    
    static bool isCardPresent() {
        return true;
    }

    static std::string readConfigFile(const char* filename = "config.json") {
        // Strip leading slash if present for native filesystem
        const char* path = (filename && filename[0] == '/') ? filename + 1 : filename;
        std::ifstream file(path);
        if (!file.is_open()) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static bool writeConfigFile(const char* filename, const char* content) {
        const char* path = (filename && filename[0] == '/') ? filename + 1 : filename;
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << content;
        return true;
    }

    static float getFloat(const char* key, float defaultValue) {
        if (store.find(key) != store.end()) return store[key];
        return defaultValue;
    }
    
    static void putFloat(const char* key, float value) {
        store[key] = value;
    }

    static int getInt(const char* key, int defaultValue) {
        if (storeInt.find(key) != storeInt.end()) return storeInt[key];
        return defaultValue;
    }

    static void putInt(const char* key, int value) {
        storeInt[key] = value;
    }
};
```

- [ ] **Step 3: Update `AmoledStoragePolicy.h` and `T-Embed RealStoragePolicy.h`**

Add ESP32 SD & SPI initialization, `isCardPresent()`, `readConfigFile()`, and `writeConfigFile()` in `AmoledStoragePolicy.h` (SPI pins: `SD_CS=38, SD_MOSI=39, SD_MISO=40, SD_SCLK=41`) and `src/Device_T_Embed_CC1101/Policies/RealStoragePolicy.h` (SPI pins: `SD_CS=13, SD_MOSI=11, SD_MISO=13, SD_SCLK=12`).

- [ ] **Step 4: Run unit tests to verify compilation**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests`
Expected: PASS.

- [ ] **Step 5: Commit**

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

- [ ] **Step 1: Write unit tests in `test/test_applogic/test_applogic.cpp`**

```cpp
void test_applogic_json_config_custom_monitors(void) {
    MockStoragePolicy::reset();
    MockStoragePolicy::configFileContent = R"json({
        "isHud": true,
        "monitors": [
            {
                "name": "Custom Delta",
                "title": "CDELTA",
                "formula": "channel(device(lap), delta_lap_time)*100.0",
                "multiplier": 0.01,
                "positive_is_good": false,
                "decimals": 2,
                "limit": 0.25
            }
        ]
    })json";

    logic.setup();
    TEST_ASSERT_TRUE(state.isHud);
    TEST_ASSERT_EQUAL(1, state.numMonitorConfigs);
    TEST_ASSERT_EQUAL_STRING("CDELTA", state.monitorConfigs[0].title);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.25f, state.monitorConfigs[0].limit);
}

void test_applogic_json_config_fallback_defaults(void) {
    MockStoragePolicy::reset();
    MockStoragePolicy::configFileContent = ""; // Empty file / missing

    logic.setup();
    TEST_ASSERT_FALSE(state.isHud);
    TEST_ASSERT_EQUAL(2, state.numMonitorConfigs);
    TEST_ASSERT_EQUAL_STRING("TIME", state.monitorConfigs[0].title);
    TEST_ASSERT_EQUAL_STRING("SPEED", state.monitorConfigs[1].title);
}

void test_applogic_json_config_auto_create(void) {
    MockStoragePolicy::reset();
    MockStoragePolicy::cardPresent = true;
    MockStoragePolicy::configFileContent = "";

    logic.setup();
    TEST_ASSERT_TRUE(MockStoragePolicy::lastWrittenFileContent.find("\"monitors\"") != std::string::npos);
    TEST_ASSERT_TRUE(MockStoragePolicy::lastWrittenFileContent.find("\"TIME\"") != std::string::npos);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests -f test_applogic`
Expected: FAIL.

- [ ] **Step 3: Implement JSON loading, fallback, and dynamic monitor registration in `AppLogic.h`**

1. Include `<ArduinoJson.h>`.
2. Add `loadConfig()`:
   - Reads `StoragePolicy::readConfigFile("/config.json")`.
   - If empty and `StoragePolicy::isCardPresent()`: call `saveDefaultConfig()`.
   - Parse with `JsonDocument doc; deserializeJson(doc, jsonStr);`.
   - If valid, parse `isHud` and `monitors` array.
   - If invalid or empty, populate default 2 monitors (`TIME` limit 0.1, `SPEED` limit 5.0) and `isHud = false`.
3. In `configureMonitors()`: iterate over `state.numMonitorConfigs` and call `addMonitorConfig` for each item.
4. Remove `toggleEditMode()`, `changeValue()`, and edit-mode checks.
5. In `processEvent()`: ignore `HW_ACTION_TOGGLE`.

- [ ] **Step 4: Run unit tests to verify they pass**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests -f test_applogic`
Expected: PASS.

- [ ] **Step 5: Commit**

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

- [ ] **Step 1: Delete `ConfigMonitorScreen.h` and `CircularConfigMonitorScreen.h`**

Run `git rm` on `src/Screens/ConfigMonitorScreen.h` and `src/Screens/CircularConfigMonitorScreen.h`.

- [ ] **Step 2: Remove edit-mode drawing in `CircularUI.h`, `CircularMonitorScreen.h`, and `MonitorScreen.h`**

Remove `drawPlusMinus` and `drawCheckmark` from `CircularUI.h`. Remove `isEditMode` branches from `CircularMonitorScreen.h` and `MonitorScreen.h`.

- [ ] **Step 3: Update `NativeViewPolicy.h`, `AmoledViewPolicy.h`, `TEmbedViewPolicy.h`, and `View.h`**

- Set `tft.setHudMode(state.isHud)` in `View::update()`.
- Screen registration:
  - Disconnected: 1 screen (`CircularDisconnectedScreen` / `DisconnectedMsgScreen`).
  - Connected: Monitor screens for `MAX_MONITORS` (up to 4).
  - No duplicate HUD screen wrapper entries in the carousel.

- [ ] **Step 4: Update `test/test_view/test_view.cpp` and run tests**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests`
Expected: PASS for all tests.

- [ ] **Step 5: Commit**

```bash
git add -u
git commit -m "refactor: delete config screens, remove edit mode from UI, apply global HUD mode"
```

---

### Task 6: End-to-End Verification & Default `config.json` Template

**Files:**
- Create: `config.json` (root of project for simulator)
- Test: `env:unit_tests` and `env:run_simulator`

- [ ] **Step 1: Create default `config.json` in project root**

```json
{
  "isHud": false,
  "monitors": [
    {
      "name": "Delta curr lap time",
      "title": "TIME",
      "formula": "channel(device(lap), delta_lap_time)*100.0",
      "multiplier": 0.01,
      "positive_is_good": false,
      "decimals": 2,
      "limit": 0.1
    },
    {
      "name": "Delta speed",
      "title": "SPEED",
      "formula": "channel(device(calc), delta_speed)*100",
      "multiplier": 0.036,
      "positive_is_good": true,
      "decimals": 1,
      "limit": 5.0
    }
  ]
}
```

- [ ] **Step 2: Run all unit tests**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests`
Expected: 100% PASS across `test_applogic`, `test_appstate`, `test_view`.

- [ ] **Step 3: Run simulator build**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe run -e run_simulator`
Expected: Compiles and links successfully.

- [ ] **Step 4: Commit**

```bash
git add config.json docs/superpowers/plans/2026-08-31-sd-json-config.md
git commit -m "docs: add plan and default config.json for simulator"
```
