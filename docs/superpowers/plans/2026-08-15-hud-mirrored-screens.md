# HUD Mirrored Screens Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add horizontally mirrored HUD versions of all screens into the screen carousel for windshield projection across T-Embed, AMOLED, and SDL2 simulator targets.

**Architecture:** A `HudScreenWrapper<DisplayPolicy>` decorator wraps any `IScreen<DisplayPolicy>` instance and sets `tft.setHudMode(true)` on `onShow` and `onUpdate`. Display policies handle hardware rotation (`setRotation(7)`) or SDL2 renderer horizontal flipping (`SDL_FLIP_HORIZONTAL`). View policies instantiate wrapped HUD versions for all connected/disconnected screens after standard screens.

**Tech Stack:** C++11, PlatformIO, TFT_eSPI (ESP32 T-Embed), Arduino_GFX (ESP32 AMOLED), SDL2 (Native Simulator), Unity (Unit Tests).

**Spec:** `docs/superpowers/specs/2026-08-15-hud-mirrored-screens-design.md`

## Global Constraints

- Event-Driven Architecture: pure UI state in `AppState`, decoupled display policies, no hardware includes in core `Screens`.
- C++ Templates: Display policies are passed as template parameters to `IScreen` and `ViewPolicy`.
- Cross-platform: Native build and unit tests run cleanly without hardware dependencies.

---

### Task 1: Add HUD Support to Display Policies

**Files:**
- Modify: `src/Device_Mock/Policies/MockDisplayPolicy.h:20-54`
- Modify: `src/Device_T_Embed_CC1101/Policies/RealDisplayPolicy.h:5-48`
- Modify: `src/Device_T_Display_S3_AMOLED/Policies/AmoledDisplayPolicy.h:10-93`
- Modify: `src/Device_Native/Policies/RealDisplayPolicy.h:10-185`

**Interfaces:**
- Consumes: None.
- Produces: `void DisplayPolicy::setHudMode(bool enabled)` across all display policies.

- [ ] **Step 1: Write unit test in `test/test_view/test_view.cpp` expecting `setHudMode` to track HUD state**

Add a test case `test_mock_display_hud_mode` in `test/test_view/test_view.cpp`:
```cpp
void test_mock_display_hud_mode(void) {
    MockDisplayPolicy display;
    TEST_ASSERT_FALSE(MockDisplayPolicy::isHud);
    display.setHudMode(true);
    TEST_ASSERT_TRUE(MockDisplayPolicy::isHud);
    display.setHudMode(false);
    TEST_ASSERT_FALSE(MockDisplayPolicy::isHud);
}
```

- [ ] **Step 2: Run unit test to verify failure**

Run: `pio test -e unit_tests`
Expected: FAIL with compilation error `'setHudMode' is not a member of 'MockDisplayPolicy'`.

- [ ] **Step 3: Implement `setHudMode` in all DisplayPolicy classes**

In `src/Device_Mock/Policies/MockDisplayPolicy.h`:
```cpp
class MockDisplayPolicy {
public:
    static bool isHud;
    static void reset() {
        lastPrint = "";
        lastFillScreenColor = 0;
        lastRects.clear();
        isHud = false;
    }
    void setHudMode(bool hud) { isHud = hud; }
...
```
Initialize `bool MockDisplayPolicy::isHud = false;` in `#ifdef PIO_UNIT_TESTING`.

In `src/Device_T_Embed_CC1101/Policies/RealDisplayPolicy.h`:
```cpp
    bool isHudMode = false;
    void setHudMode(bool hud) {
        if (isHudMode != hud) {
            isHudMode = hud;
            tft.setRotation(hud ? 7 : 3);
        }
    }
```

In `src/Device_T_Display_S3_AMOLED/Policies/AmoledDisplayPolicy.h`:
```cpp
    bool isHudMode = false;
    void setHudMode(bool hud) {
        if (isHudMode != hud) {
            isHudMode = hud;
            gfx->setRotation(hud ? 5 : 1);
        }
    }
```

In `src/Device_Native/Policies/RealDisplayPolicy.h`:
```cpp
    bool isHudMode = false;
    void setHudMode(bool hud) {
        isHudMode = hud;
    }
```
And in `RealDisplayPolicy::flush()` in `Device_Native/Policies/RealDisplayPolicy.h`:
```cpp
    void flush() {
        if (isHudMode) {
            // Create target texture if needed and render with SDL_FLIP_HORIZONTAL
            // or render window flipped:
            SDL_RenderPresent(renderer);
        } else {
            SDL_RenderPresent(renderer);
        }
    }
```

- [ ] **Step 4: Run unit test to verify pass**

Run: `pio test -e unit_tests`
Expected: PASS.

- [ ] **Step 5: Commit Task 1**

```bash
git add src/Device_Mock/Policies/MockDisplayPolicy.h src/Device_T_Embed_CC1101/Policies/RealDisplayPolicy.h src/Device_T_Display_S3_AMOLED/Policies/AmoledDisplayPolicy.h src/Device_Native/Policies/RealDisplayPolicy.h test/test_view/test_view.cpp
git commit -m "feat: add setHudMode to all DisplayPolicy implementations"
```

---

### Task 2: Create `HudScreenWrapper<DisplayPolicy>` and Update `View.h`

**Files:**
- Create: `src/Screens/HudScreenWrapper.h`
- Modify: `src/View.h:90-110`
- Test: `test/test_view/test_view.cpp`

**Interfaces:**
- Consumes: `IScreen<DisplayPolicy>`, `DisplayPolicy::setHudMode(bool)`
- Produces: `HudScreenWrapper<DisplayPolicy>` decorator screen class.

- [ ] **Step 1: Write test in `test/test_view/test_view.cpp` for `HudScreenWrapper`**

Add test function `test_hud_screen_wrapper`:
```cpp
void test_hud_screen_wrapper(void) {
    MockDisplayPolicy::reset();
    MonitorScreen<MockDisplayPolicy> innerMonitor{0};
    HudScreenWrapper<MockDisplayPolicy> hudWrapper(&innerMonitor);

    state.isConnected = true;
    state.speedLimit = 5.0f;
    state.addMonitor("M1", 1.0f, "SPEED", true, 1, &state.speedLimit);
    state.setMonitorValue(0, 10);

    TEST_ASSERT_FALSE(MockDisplayPolicy::isHud);
    hudWrapper.onUpdate(view.tft, state);
    TEST_ASSERT_TRUE(MockDisplayPolicy::isHud);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("SPEED") != std::string::npos);
}
```

- [ ] **Step 2: Run unit test to verify failure**

Run: `pio test -e unit_tests`
Expected: FAIL with compilation error `'HudScreenWrapper' was not declared in this scope`.

- [ ] **Step 3: Create `src/Screens/HudScreenWrapper.h` and update `View.h`**

Create `src/Screens/HudScreenWrapper.h`:
```cpp
#pragma once
#include "IScreen.h"

template <typename DisplayPolicy>
class HudScreenWrapper : public IScreen<DisplayPolicy> {
private:
    IScreen<DisplayPolicy>* innerScreen;

public:
    explicit HudScreenWrapper(IScreen<DisplayPolicy>* screen) : innerScreen(screen) {}

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.setHudMode(true);
        if (innerScreen) {
            innerScreen->onShow(tft, state);
        }
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        tft.setHudMode(true);
        if (innerScreen) {
            innerScreen->onUpdate(tft, state);
        }
    }
};
```

In `src/View.h`, ensure default `tft.setHudMode(false)` is set in `View::update()` before updating screens if not wrapped:
```cpp
    void update() {
        tft.setHudMode(false);
        int currentIdx = state.isConnected ? state.currentScreenIndex : state.disconnectedScreenIndex;
        ...
```

- [ ] **Step 4: Run unit test to verify pass**

Run: `pio test -e unit_tests`
Expected: PASS.

- [ ] **Step 5: Commit Task 2**

```bash
git add src/Screens/HudScreenWrapper.h src/View.h test/test_view/test_view.cpp
git commit -m "feat: add HudScreenWrapper decorator and update View for HUD mode reset"
```

---

### Task 3: Register HUD Screens in View Policies

**Files:**
- Modify: `src/Device_T_Embed_CC1101/Policies/TEmbedViewPolicy.h:1-33`
- Modify: `src/Device_T_Display_S3_AMOLED/Policies/AmoledViewPolicy.h:1-33`
- Modify: `src/Device_Native/Policies/NativeViewPolicy.h:1-53`
- Test: `test/test_view/test_view.cpp`

**Interfaces:**
- Consumes: `HudScreenWrapper<DisplayPolicy>`, `View::addConnectedScreen`, `View::addDisconnectedScreen`
- Produces: Updated `setupScreens` adding HUD versions after standard screens.

- [ ] **Step 1: Write test for screen carousel counts in `test/test_view/test_view.cpp`**

```cpp
void test_hud_screen_registration(void) {
    state.reset();
    View<MockDisplayPolicy, MockHWPolicy> mockView(state);
    NativeViewPolicy<MockDisplayPolicy> policy(state);
    policy.setupScreens(mockView);

    // 3 circ connected + 3 std connected + 3 circ HUD + 3 std HUD = 12 connected screens
    TEST_ASSERT_EQUAL(12, mockView.getNumConnectedScreens());
    // 1 msg + 2 circ config + 2 std config + 1 msg HUD + 2 circ config HUD + 2 std config HUD = 10 disconnected screens
    TEST_ASSERT_EQUAL(10, mockView.getNumDisconnectedScreens());
}
```

- [ ] **Step 2: Run unit test to verify failure**

Run: `pio test -e unit_tests`
Expected: FAIL (counts mismatch, currently 6 connected and 5 disconnected).

- [ ] **Step 3: Add wrapped HUD screens to `TEmbedViewPolicy`, `AmoledViewPolicy`, and `NativeViewPolicy`**

In `src/Device_T_Embed_CC1101/Policies/TEmbedViewPolicy.h`:
Add wrapped HUD screen members:
```cpp
    HudScreenWrapper<DisplayPolicy> hudMonitor0{&monitor0};
    HudScreenWrapper<DisplayPolicy> hudMonitor1{&monitor1};
    HudScreenWrapper<DisplayPolicy> hudDualMonitor{&dualMonitor};
    HudScreenWrapper<DisplayPolicy> hudDisconnectedMsg{&disconnectedMsg};
    HudScreenWrapper<DisplayPolicy> hudConfigSpeed{&configSpeed};
    HudScreenWrapper<DisplayPolicy> hudConfigTime{&configTime};
```
In `setupScreens`:
Add standard screens first, then HUD screens:
```cpp
        appView.addConnectedScreen(&monitor0);
        appView.addConnectedScreen(&monitor1);
        appView.addConnectedScreen(&dualMonitor);
        appView.addConnectedScreen(&hudMonitor0);
        appView.addConnectedScreen(&hudMonitor1);
        appView.addConnectedScreen(&hudDualMonitor);

        appView.addDisconnectedScreen(&disconnectedMsg);
        appView.addDisconnectedScreen(&configSpeed);
        appView.addDisconnectedScreen(&configTime);
        appView.addDisconnectedScreen(&hudDisconnectedMsg);
        appView.addDisconnectedScreen(&hudConfigSpeed);
        appView.addDisconnectedScreen(&hudConfigTime);
```

Apply equivalent additions to `AmoledViewPolicy.h` and `NativeViewPolicy.h`.

- [ ] **Step 4: Run unit test to verify pass**

Run: `pio test -e unit_tests`
Expected: PASS.

- [ ] **Step 5: Commit Task 3**

```bash
git add src/Device_T_Embed_CC1101/Policies/TEmbedViewPolicy.h src/Device_T_Display_S3_AMOLED/Policies/AmoledViewPolicy.h src/Device_Native/Policies/NativeViewPolicy.h test/test_view/test_view.cpp
git commit -m "feat: register grouped HUD screens in all device ViewPolicies"
```

---

### Task 4: Verification and Simulator Test

**Files:** None (Verification run).

- [ ] **Step 1: Execute all unit tests**

Run: `pio test -e unit_tests`
Expected: All tests PASS.

- [ ] **Step 2: Build SDL2 native simulator**

Run: `pio run -e run_simulator`
Expected: Clean compilation and successful launch of simulator.
