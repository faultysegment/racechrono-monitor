# Custom Monitors and Screen Composition Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Allow users to define custom RaceChrono monitors with custom string IDs and compose arbitrary screens (`single`, `dual`) referencing those IDs from `config.json`.

**Architecture:** `AppState` holds `MonitorConfig` (with string `id`) and `ScreenConfig` (with resolved monitor indices). `AppLogic::loadConfig()` parses `monitors` and `screens` from JSON and resolves string IDs to array indices (with automatic fallback if `screens` is omitted). ViewPolicies dynamically bind pre-allocated screen instances (`MonitorScreen`, `CircularMonitorScreen`, `DualMonitorScreen`) to the configured monitor indices without runtime heap allocations.

**Tech Stack:** C++11, PlatformIO, ArduinoJson v7, FreeRTOS, TFT_eSPI, Arduino_GFX, SDL2, Unity.

**Spec:** [docs/superpowers/specs/2026-08-31-custom-screens-and-monitors-design.md](file:///c:/Users/faultysegment/Documents/PlatformIO/Projects/racechrono-monitor/docs/superpowers/specs/2026-08-31-custom-screens-and-monitors-design.md)

## Global Constraints
- Core classes (`AppLogic.h`, `View.h`, `AppState.h`) must never include hardware-specific headers (`<Arduino.h>`, `<TFT_eSPI.h>`, `<SD.h>`, `<SPI.h>`).
- Cross-platform native builds (unit tests and SDL2 simulator) must compile cleanly without host-specific paths.
- Default fallback when `screens` is omitted: generate $N$ `single` screens, plus 1 `dual` screen if $N \ge 2$.

---

### Task 1: Update `AppState.h` Data Structures

**Files:**
- Modify: `src/AppState.h`
- Test: `test/test_appstate/test_appstate.cpp`

**Interfaces:**
- Consumes: None
- Produces: `MonitorConfig::id`, `enum class ScreenType`, `struct ScreenConfig`, `state.screenConfigs`, `state.numScreenConfigs`, `state.findMonitorIndexById(const char*)`.

- [ ] **Step 1: Write unit tests in `test/test_appstate/test_appstate.cpp`**

```cpp
void test_appstate_screen_configs(void) {
    AppState s;
    s.reset();
    TEST_ASSERT_EQUAL(0, s.numMonitorConfigs);
    TEST_ASSERT_EQUAL(0, s.numScreenConfigs);

    MonitorConfig m1;
    strncpy(m1.id, "lap_delta", sizeof(m1.id));
    strncpy(m1.title, "TIME", sizeof(m1.title));
    s.addMonitorConfig(m1);

    MonitorConfig m2;
    strncpy(m2.id, "speed_delta", sizeof(m2.id));
    strncpy(m2.title, "SPEED", sizeof(m2.title));
    s.addMonitorConfig(m2);

    TEST_ASSERT_EQUAL(0, s.findMonitorIndexById("lap_delta"));
    TEST_ASSERT_EQUAL(1, s.findMonitorIndexById("speed_delta"));
    TEST_ASSERT_EQUAL(-1, s.findMonitorIndexById("non_existent"));

    ScreenConfig sc1{ScreenType::SINGLE, 0, -1};
    ScreenConfig sc2{ScreenType::DUAL, 0, 1};
    s.addScreenConfig(sc1);
    s.addScreenConfig(sc2);

    TEST_ASSERT_EQUAL(2, s.numScreenConfigs);
    TEST_ASSERT_EQUAL(ScreenType::SINGLE, s.screenConfigs[0].type);
    TEST_ASSERT_EQUAL(0, s.screenConfigs[0].primaryMonitorIndex);
    TEST_ASSERT_EQUAL(ScreenType::DUAL, s.screenConfigs[1].type);
    TEST_ASSERT_EQUAL(1, s.screenConfigs[1].secondaryMonitorIndex);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests -f test_appstate`
Expected: FAIL (missing `ScreenConfig`, `findMonitorIndexById`, etc.).

- [ ] **Step 3: Update `src/AppState.h`**

Update `MAX_MONITORS = 8`, `MAX_SCREENS = 8`, `MONITOR_ID_MAX_LEN = 16`, add `id` to `MonitorConfig`, define `ScreenType` and `ScreenConfig`, add `screenConfigs`, `numScreenConfigs`, `addScreenConfig`, `clearScreenConfigs`, and `findMonitorIndexById`.

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
    int primaryMonitorIndex;
    int secondaryMonitorIndex;
};
```

- [ ] **Step 4: Run unit tests to verify they pass**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests -f test_appstate`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/AppState.h test/test_appstate/test_appstate.cpp
git commit -m "feat: add ScreenConfig and findMonitorIndexById to AppState"
```

---

### Task 2: Update Screen Renderers with Configurable Monitor Indices

**Files:**
- Modify: `src/Screens/MonitorScreen.h`
- Modify: `src/Screens/CircularMonitorScreen.h`
- Modify: `src/Screens/DualMonitorScreen.h`

**Interfaces:**
- Consumes: `AppState`
- Produces: `MonitorScreen::setMonitorIndex(int)`, `CircularMonitorScreen::setMonitorIndex(int)`, `DualMonitorScreen::setMonitors(int, int)`.

- [ ] **Step 1: Update `MonitorScreen.h` and `CircularMonitorScreen.h`**

Add `setMonitorIndex(int idx)`:
```cpp
void setMonitorIndex(int idx) {
    mIdx = idx;
}
```

- [ ] **Step 2: Update `DualMonitorScreen.h`**

Add members `mTopIdx` and `mBtmIdx`, constructor `DualMonitorScreen(int topIdx = 0, int btmIdx = 1)`, and `setMonitors(int topIdx, int btmIdx)`:
```cpp
template <typename DisplayPolicy>
class DualMonitorScreen : public IScreen<DisplayPolicy> {
    int mTopIdx;
    int mBtmIdx;

public:
    DualMonitorScreen(int topIdx = 0, int btmIdx = 1) : mTopIdx(topIdx), mBtmIdx(btmIdx) {}

    void setMonitors(int topIdx, int btmIdx) {
        mTopIdx = topIdx;
        mBtmIdx = btmIdx;
    }

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        UI<DisplayPolicy> ui(tft);
        ui.begin();

        float barH = 0.23f; 
        
        // Draw Top Monitor
        if (state.nextMonitorId > mTopIdx && mTopIdx >= 0) {
            drawMonitor(ui, state, mTopIdx, 0.05f, 0.23f, barH);
        }
        
        // Draw Bottom Monitor
        if (state.nextMonitorId > mBtmIdx && mBtmIdx >= 0) {
            drawMonitor(ui, state, mBtmIdx, 0.52f, 0.70f, barH);
        }
    }
```

- [ ] **Step 3: Run unit tests to verify compilation**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests`
Expected: PASS.

- [ ] **Step 4: Commit**

```bash
git add src/Screens/MonitorScreen.h src/Screens/CircularMonitorScreen.h src/Screens/DualMonitorScreen.h
git commit -m "feat: make DualMonitorScreen, MonitorScreen and CircularMonitorScreen monitor indices configurable"
```

---

### Task 3: Implement JSON Parsing for Custom Screens in `AppLogic.h`

**Files:**
- Modify: `src/AppLogic.h`
- Test: `test/test_applogic/test_applogic.cpp`

**Interfaces:**
- Consumes: `StoragePolicy`, `ArduinoJson`, `AppState::MonitorConfig`, `AppState::ScreenConfig`.
- Produces: `AppLogic::loadConfig()` with `id` and `screens` parsing + fallback screen generation, updated `saveDefaultConfig()`.

- [ ] **Step 1: Write unit tests in `test/test_applogic/test_applogic.cpp`**

```cpp
void test_applogic_json_custom_screens(void) {
    setUp();
    MockStoragePolicy::reset();
    MockStoragePolicy::configFileContent = R"json({
        "isHud": false,
        "monitors": [
            {
                "id": "delta_time",
                "title": "TIME",
                "formula": "channel(device(lap), delta_lap_time)*100.0",
                "multiplier": 0.01,
                "positive_is_good": false,
                "decimals": 2,
                "limit": 0.5
            },
            {
                "id": "delta_speed",
                "title": "SPEED",
                "formula": "channel(device(calc), delta_speed)*100",
                "multiplier": 0.036,
                "positive_is_good": true,
                "decimals": 1,
                "limit": 1.0
            }
        ],
        "screens": [
            { "type": "dual", "top": "delta_speed", "bottom": "delta_time" },
            { "type": "single", "monitor": "delta_speed" }
        ]
    })json";

    logic.setup();
    TEST_ASSERT_EQUAL(2, state.numMonitorConfigs);
    TEST_ASSERT_EQUAL(2, state.numScreenConfigs);

    // Screen 0 is dual with top=speed (idx 1) and bottom=time (idx 0)
    TEST_ASSERT_EQUAL(ScreenType::DUAL, state.screenConfigs[0].type);
    TEST_ASSERT_EQUAL(1, state.screenConfigs[0].primaryMonitorIndex);
    TEST_ASSERT_EQUAL(0, state.screenConfigs[0].secondaryMonitorIndex);

    // Screen 1 is single with monitor=speed (idx 1)
    TEST_ASSERT_EQUAL(ScreenType::SINGLE, state.screenConfigs[1].type);
    TEST_ASSERT_EQUAL(1, state.screenConfigs[1].primaryMonitorIndex);
}

void test_applogic_json_fallback_screens_when_omitted(void) {
    setUp();
    MockStoragePolicy::reset();
    MockStoragePolicy::configFileContent = R"json({
        "isHud": false,
        "monitors": [
            {
                "id": "m1",
                "title": "M1",
                "formula": "formula1",
                "multiplier": 1.0,
                "positive_is_good": false,
                "decimals": 1,
                "limit": 1.0
            }
        ]
    })json";

    logic.setup();
    TEST_ASSERT_EQUAL(1, state.numMonitorConfigs);
    TEST_ASSERT_EQUAL(1, state.numScreenConfigs);
    TEST_ASSERT_EQUAL(ScreenType::SINGLE, state.screenConfigs[0].type);
    TEST_ASSERT_EQUAL(0, state.screenConfigs[0].primaryMonitorIndex);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests -f test_applogic`
Expected: FAIL.

- [ ] **Step 3: Update `AppLogic.h`**

1. Parse `m["id"]` into `cfg.id`.
2. Parse `screens` array:
   - For `"single"`: resolve `"monitor"` `id` using `state.findMonitorIndexById()`. If valid, add `ScreenConfig{ScreenType::SINGLE, idx, -1}`.
   - For `"dual"`: resolve `"top"` and `"bottom"` `id`s. If both valid, add `ScreenConfig{ScreenType::DUAL, topIdx, btmIdx}`.
3. If `state.numScreenConfigs == 0` (omitted or empty):
   - For each monitor $i \in [0, \text{numMonitorConfigs}-1]$: add `ScreenConfig{ScreenType::SINGLE, i, -1}`.
   - If $\text{numMonitorConfigs} \ge 2$: add `ScreenConfig{ScreenType::DUAL, 0, 1}`.
4. Update `saveDefaultConfig()` and `loadDefaultConfig()` to output `id` for monitors and the default `screens` array.

- [ ] **Step 4: Run unit tests to verify they pass**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests -f test_applogic`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/AppLogic.h test/test_applogic/test_applogic.cpp
git commit -m "feat: parse custom screens and monitor IDs in AppLogic"
```

---

### Task 4: Update View Policies for Dynamic Screen Composition

**Files:**
- Modify: `src/Device_Native/Policies/NativeViewPolicy.h`
- Modify: `src/Device_T_Display_S3_AMOLED/Policies/AmoledViewPolicy.h`
- Modify: `src/Device_T_Embed_CC1101/Policies/TEmbedViewPolicy.h`
- Test: `test/test_view/test_view.cpp`

**Interfaces:**
- Consumes: `AppState::screenConfigs`, `AppState::numScreenConfigs`.
- Produces: `setupScreens(appView, state)` configuring screens based on `state.screenConfigs`.

- [ ] **Step 1: Write unit tests in `test/test_view/test_view.cpp`**

```cpp
void test_view_custom_screen_composition(void) {
    state.reset();
    state.numMonitorConfigs = 2;
    state.numScreenConfigs = 2;
    state.screenConfigs[0] = ScreenConfig{ScreenType::DUAL, 1, 0}; // top: SPEED (idx 1), btm: TIME (idx 0)
    state.screenConfigs[1] = ScreenConfig{ScreenType::SINGLE, 1, -1}; // single: SPEED (idx 1)

    View<MockDisplayPolicy, MockHWPolicy> mockView(state);
    NativeViewPolicy<MockDisplayPolicy> policy(state);
    policy.setupScreens(mockView, state);

    TEST_ASSERT_EQUAL(2, mockView.getNumConnectedScreens());
    TEST_ASSERT_EQUAL(1, mockView.getNumDisconnectedScreens());
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests -f test_view`
Expected: FAIL.

- [ ] **Step 3: Update `TEmbedViewPolicy.h`, `AmoledViewPolicy.h`, and `NativeViewPolicy.h`**

1. In `TEmbedViewPolicy.h`:
   - `MonitorScreen<DisplayPolicy> singleScreens[MAX_SCREENS];`
   - `DualMonitorScreen<DisplayPolicy> dualScreens[MAX_SCREENS];`
   - In `setupScreens`: iterate `state.screenConfigs`, configure `singleScreens[i]` or `dualScreens[i]`, and add to `appView`.
2. In `AmoledViewPolicy.h`:
   - `CircularMonitorScreen<DisplayPolicy> singleScreens[MAX_SCREENS];`
   - In `setupScreens`: iterate `state.screenConfigs`, configure `singleScreens[i]` with `primaryMonitorIndex`, and add to `appView`.
3. In `NativeViewPolicy.h`:
   - Pre-allocate arrays of `MonitorScreen`, `CircularMonitorScreen`, and `DualMonitorScreen`.
   - In `setupScreens`: iterate `state.screenConfigs` and add configured screens to `appView`.

- [ ] **Step 4: Run unit tests to verify they pass**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests`
Expected: 100% PASS across all suites.

- [ ] **Step 5: Commit**

```bash
git add src/Device_Native/Policies/NativeViewPolicy.h src/Device_T_Display_S3_AMOLED/Policies/AmoledViewPolicy.h src/Device_T_Embed_CC1101/Policies/TEmbedViewPolicy.h test/test_view/test_view.cpp
git commit -m "feat: compose screens dynamically in ViewPolicies from screenConfigs"
```

---

### Task 5: End-to-End Verification & `config.json` Template Update

**Files:**
- Modify: `config.json`
- Test: `env:unit_tests`, `env:run_simulator`, `env:T_Embed_CC1101`, `env:T_Display_S3_AMOLED`

- [ ] **Step 1: Update `config.json` in project root**

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

- [ ] **Step 2: Run all unit tests**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe test -e unit_tests`
Expected: 100% PASS.

- [ ] **Step 3: Run SDL2 Simulator build**

Run: `C:\Users\faultysegment\.platformio\penv\Scripts\platformio.exe run -e run_simulator`
Expected: Compiles and links cleanly.

- [ ] **Step 4: Commit**

```bash
git add config.json docs/superpowers/plans/2026-08-31-custom-screens-and-monitors.md
git commit -m "feat: update config.json template for custom monitors and screens"
```
