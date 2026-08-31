# Custom Monitors and Screen Composition Design Spec

## Overview
This specification details the architecture and implementation for defining custom RaceChrono monitors with custom string IDs and composing custom screens from those monitors in `config.json`.

---

## 1. `config.json` Schema

The configuration file allows defining an arbitrary list of monitors and an ordered list of screens that reference those monitors by `id`.

```json
{
  "isHud": false,
  "monitors": [
    {
      "id": "lap_delta",
      "title": "TIME",
      "formula": "channel(device(lap), delta_lap_time)*100.0",
      "multiplier": 0.01,
      "positive_is_good": false,
      "decimals": 2,
      "limit": 0.5
    },
    {
      "id": "speed_delta",
      "title": "SPEED",
      "formula": "channel(device(calc), delta_speed)*100",
      "multiplier": 0.036,
      "positive_is_good": true,
      "decimals": 1,
      "limit": 1.0
    }
  ],
  "screens": [
    { "type": "single", "monitor": "lap_delta" },
    { "type": "single", "monitor": "speed_delta" },
    { "type": "dual", "top": "lap_delta", "bottom": "speed_delta" }
  ]
}
```

### Fields

#### `monitors` Array
- `id` (string, max 16 chars): Unique monitor identifier used for screen binding.
- `title` (string, max 16 chars): Text label rendered on the screen (e.g. `"TIME"`).
- `formula` (string, max 128 chars): RaceChrono channel query formula.
- `multiplier` (float): Value multiplier.
- `positive_is_good` (boolean): `true` if positive delta is green, `false` if positive is red.
- `decimals` (integer): Decimal precision (1 or 2).
- `limit` (float): Target scale limit.

#### `screens` Array
- `type` (string): Screen layout type: `"single"` or `"dual"`.
- `monitor` (string, required for `single`): The `id` of the monitor to display.
- `top` (string, required for `dual`): The `id` of the monitor displayed on top.
- `bottom` (string, required for `dual`): The `id` of the monitor displayed on bottom.

---

## 2. Fallback & Default Behavior

1. **Missing or empty `screens` section:**
   - If `monitors` has $N \ge 1$ entries, automatically generate $N$ `single` screens (one for each monitor).
   - If $N \ge 2$, append a `dual` screen displaying `monitors[0]` (top) and `monitors[1]` (bottom).
2. **Missing or empty `/config.json` on MicroSD card:**
   - Automatically write the default template containing 2 monitors (`lap_delta`, `speed_delta`) and 3 screens (`single`, `single`, `dual`).
   - Use these default structures immediately in memory.
3. **Invalid Monitor ID in `screens`:**
   - If a screen references an unknown `id`, that screen is skipped during registration with a warning log.

---

## 3. Data Structures in `AppState.h`

```cpp
static constexpr int MAX_MONITORS = 8;
static constexpr int MAX_SCREENS = 8;
static constexpr int MONITOR_ID_MAX_LEN = 16;

struct MonitorConfig {
    char id[MONITOR_ID_MAX_LEN + 1];
    char title[16];
    char formula[128];
    float multiplier;
    bool positiveIsGood;
    int decimals;
    float limit;
};

enum class ScreenType {
    SINGLE,
    DUAL
};

struct ScreenConfig {
    ScreenType type;
    int primaryMonitorIndex;   // Index into state.monitors (0 .. MAX_MONITORS - 1)
    int secondaryMonitorIndex; // For DUAL screens (-1 if unused)
};
```

In `AppState`:
- `MonitorConfig monitorConfigs[MAX_MONITORS]; int numMonitorConfigs = 0;`
- `ScreenConfig screenConfigs[MAX_SCREENS]; int numScreenConfigs = 0;`
- `int findMonitorIndexById(const char* id) const`: Look up monitor index by string `id`, returns `-1` if not found.

---

## 4. Screen Rendering Components

### 4.1 `MonitorScreen` & `CircularMonitorScreen`
- Constructor: `MonitorScreen(int monitorIndex = 0)`
- Setter: `void setMonitorIndex(int idx) { mIdx = idx; }`
- Renders `state.monitors[mIdx]`.

### 4.2 `DualMonitorScreen`
- Constructor: `DualMonitorScreen(int topIdx = 0, int bottomIdx = 1)`
- Setter: `void setMonitors(int topIdx, int bottomIdx) { mTopIdx = topIdx; mBtmIdx = bottomIdx; }`
- Renders `state.monitors[mTopIdx]` on the top half and `state.monitors[mBtmIdx]` on the bottom half.

---

## 5. View Policies & Static Screen Allocation

To avoid dynamic heap allocations during runtime:
- `TEmbedViewPolicy`:
  - Contains `MonitorScreen singleScreens[MAX_SCREENS];`
  - Contains `DualMonitorScreen dualScreens[MAX_SCREENS];`
  - In `setupScreens(appView, state)`:
    - Iterates over `state.screenConfigs` up to `state.numScreenConfigs`.
    - If `SINGLE`: `singleScreens[i].setMonitorIndex(sc.primaryMonitorIndex)`, adds to `appView`.
    - If `DUAL`: `dualScreens[i].setMonitors(sc.primaryMonitorIndex, sc.secondaryMonitorIndex)`, adds to `appView`.
- `AmoledViewPolicy`:
  - Contains `CircularMonitorScreen singleScreens[MAX_SCREENS];`
  - In `setupScreens(appView, state)`:
    - If `SINGLE` or `DUAL`: `singleScreens[i].setMonitorIndex(sc.primaryMonitorIndex)`, adds to `appView`.
- `NativeViewPolicy`:
  - Contains pools of circular and rectangular screens. Configured screens are registered dynamically according to `state.screenConfigs`.

---

## 6. Verification Plan

1. **Unit Tests (`env:unit_tests`)**:
   - `test_appstate_custom_monitors_and_screens`: Verify `findMonitorIndexById`, `MonitorConfig`, and `ScreenConfig`.
   - `test_applogic_json_custom_screens`: Test parsing custom screens referencing monitor IDs.
   - `test_applogic_json_fallback_screens`: Test fallback screen generation when `screens` is omitted.
   - `test_view_custom_screen_registration`: Test `setupScreens` binding arbitrary monitor pairs to `DualMonitorScreen` and `MonitorScreen`.
2. **Native Simulator (`env:run_simulator`)**:
   - Verify multi-screen carousel with custom monitors defined in `config.json`.
3. **ESP32 Firmware Targets**:
   - Verify `pio run -e T_Embed_CC1101` and `pio run -e T_Display_S3_AMOLED`.
