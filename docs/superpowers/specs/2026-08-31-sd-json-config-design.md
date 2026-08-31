# MicroSD JSON Configuration & On-Device Edit Mode Elimination Specification

## 1. Overview
This specification details the transition from on-device parameter editing and hardcoded monitor channels to reading an external `config.json` file from a MicroSD card (or local disk in simulation). It also introduces global `isHud` configuration support from JSON, default configuration creation when the file is absent on an inserted SD card, and removes all on-device edit-mode logic and screens.

---

## 2. Configuration Schema (`config.json`)

### 2.1 JSON Format
The configuration file is located at `/config.json` on the MicroSD card (or `config.json` in the working directory for the PC simulator).

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

### 2.2 Default Fallback & Auto-Creation
- **Fallback**: If MicroSD card is absent, unreadable, empty, or contains invalid JSON, the system loads the built-in default list of 2 monitors (`TIME` and `SPEED`) and `isHud = false`.
- **Auto-Creation**: If an SD card is present but `/config.json` does not exist, `StoragePolicy` creates `/config.json` containing the default JSON template so the user can easily edit it.

---

## 3. Data Structures & State (`src/AppState.h`)

### 3.1 `MonitorConfig` Definition
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
```

### 3.2 `AppState` Updates
- Holds loaded monitor configurations: `MonitorConfig monitorConfigs[MAX_MONITORS]; int numMonitorConfigs;`
- Holds global HUD flag: `bool isHud;`
- Removed fields: `bool isEditMode;`
- Retained fields: `speedLimit`, `timeLimit` (for backward compatibility / quick access), `currentScreenIndex`, `disconnectedScreenIndex`, `batteryPercent`, `monitors[MAX_MONITORS]`.

---

## 4. Storage Subsystem (`StoragePolicy`)

### 4.1 Interface Methods
All `StoragePolicy` implementations must provide:
```cpp
static bool isCardPresent();
static std::string readConfigFile(const char* filename = "/config.json");
static bool writeConfigFile(const char* filename, const char* content);
static int getInt(const char* key, int defaultValue);
static void putInt(const char* key, int value);
```

### 4.2 Target Platforms:
- **`AmoledStoragePolicy` (`src/Device_T_Display_S3_AMOLED/Policies/AmoledStoragePolicy.h`)**:
  - Uses ESP32 `SD.h` and `SPI.h` with pins: `SD_CS = 38, SD_MOSI = 39, SD_MISO = 40, SD_SCLK = 41`.
  - Initializes SPI bus and mounts SD card.
- **`RealStoragePolicy` (T-Embed CC1101, `src/Device_T_Embed_CC1101/Policies/RealStoragePolicy.h`)**:
  - Uses ESP32 `SD.h` and `SPI.h` with T-Embed pins (`SD_CS = 13, SD_MOSI = 11, SD_MISO = 13, SD_SCLK = 12`).
- **`RealStoragePolicy` (Native Simulator, `src/Device_Native/Policies/RealStoragePolicy.h`)**:
  - Reads/writes `config.json` via standard `std::ifstream` and `std::ofstream`.
- **`MockStoragePolicy` (`src/Device_Mock/Policies/MockStoragePolicy.h`)**:
  - In-memory mock string `mockConfigFileContent`, simulating card presence, missing file, empty file, and write capture.

---

## 5. Application Logic (`src/AppLogic.h`)

### 5.1 Initialization (`setup()`)
1. Reads `/config.json` using `StoragePolicy::readConfigFile("/config.json")`.
2. If file missing/empty and card is present $\implies$ writes default template via `StoragePolicy::writeConfigFile(...)`.
3. Parses JSON with `ArduinoJson` (v7):
   - Reads `isHud` boolean $\to$ sets `state.isHud`.
   - Reads `monitors` array $\to$ populates `state.monitorConfigs` and `state.numMonitorConfigs`.
   - Populates `state.timeLimit` and `state.speedLimit` from monitor limits.
4. Restores `last_screen` and applies `state.isHud` to DisplayPolicy / View.

### 5.2 Monitor Registration (`configureMonitors()`)
- Clears active monitors: `state.resetMonitors()`.
- Iterates over `state.monitorConfigs`:
  ```cpp
  for (int i = 0; i < state.numMonitorConfigs; i++) {
      addMonitorConfig(state.monitorConfigs[i].name,
                       state.monitorConfigs[i].formula,
                       state.monitorConfigs[i].multiplier,
                       state.monitorConfigs[i].title,
                       state.monitorConfigs[i].positiveIsGood,
                       state.monitorConfigs[i].decimals,
                       &state.monitors[i].limit);
  }
  ```

### 5.3 Removal of Edit Mode Logic
- Removed: `toggleEditMode()`, `changeValue()`.
- `EventType::HW_ACTION_TOGGLE`: No-op (ignored).
- `EventType::HW_NAV_DELTA`: Always invokes `changePage()`.

---

## 6. View & UI Subsystem

### 6.1 Screen Cleanup
- **Deleted Files**:
  - `src/Screens/ConfigMonitorScreen.h`
  - `src/Screens/CircularConfigMonitorScreen.h`
- **`CircularUI.h`**:
  - Removed `drawPlusMinus()` and `drawCheckmark()`.
- **`CircularMonitorScreen.h` / `MonitorScreen.h`**:
  - Removed all `if (state.isEditMode) { ... }` branches.

### 6.2 View Policies & Global HUD Mode
- Since `isHud` is globally defined by configuration, duplicate wrapped HUD screen registrations (`HudScreenWrapper`) in the carousel are eliminated.
- In `View::update()`, `tft.setHudMode(state.isHud)` is set globally.
- Screen carousel contains strictly:
  - Disconnected state: 1 screen (`DisconnectedMsgScreen` or `CircularDisconnectedScreen`).
  - Connected state: Monitor screens corresponding to configured monitors (`circMonitor0`, `circMonitor1`, etc.).

---

## 7. Verification Plan

### 7.1 Automated Unit Tests (`env:unit_tests`)
- **`test_applogic_json_config_parsing`**: Verifies parsing custom monitor names, formulas, limits, and `isHud` true/false.
- **`test_applogic_json_config_fallback_defaults`**: Verifies fallback to 2 default monitors on empty/missing/invalid JSON.
- **`test_applogic_json_config_auto_create`**: Verifies writing default template when card is present but file is absent.
- **`test_view_global_hud_mode`**: Verifies `tft.setHudMode(state.isHud)` is respected across screens without carousel duplication.

### 7.2 Manual Verification (PC Simulator & Hardware)
- Launch `pio run -e run_simulator` with custom `config.json` (e.g. 3 monitors or `isHud: true`) and verify layout, navigation, and BLE updates.
