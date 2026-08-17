# AMOLED Subsystem Refactoring Specification

## 1. Overview
This specification defines the clean refactoring of the LilyGO T-Display S3 AMOLED device layer in `src/Device_T_Display_S3_AMOLED/`. The goal is to eliminate global anonymous state, remove repetitive runtime canvas checks, formalize gesture recognition with named geometry constants, and establish a clear lifecycle for PSRAM double buffering and dual-core FreeRTOS tasks.

---

## 2. Architecture & Components

### 2.1 `AmoledDisplayPolicy` (`src/Device_T_Display_S3_AMOLED/Policies/AmoledDisplayPolicy.h`)
- **Ownership & Lifecycle**:
  - Owns `Arduino_DataBus *bus`, `Arduino_GFX *gfx`, and `Arduino_Canvas *canvas` as private static members.
  - `init()` runs inside `setup()` (where ESP32-S3 PSRAM is initialized) and constructs `bus` $\to$ `gfx` $\to$ `canvas` once.
  - `canvas` is guaranteed valid for the entire runtime lifecycle using `GFX_SKIP_OUTPUT_BEGIN`.
- **Zero-Boilerplate Direct Dispatch**:
  - Drawing primitives (`fillScreen`, `setCursor`, `setTextWrap`, `setTextSize`, `setTextColor`, `print`, `println`, `textWidth`, `fillRect`, `fillCircle`, `drawFastHLine`, `drawFastVLine`) directly dispatch to `canvas->...` in a single line without dynamic checks.
  - `flush()` calls `canvas->flush()` for atomic DMA blits over QSPI.
- **Optimized Arc Triangle Mesh**:
  - `fillArc` performs direct triangle-mesh generation into `canvas` with a $2^\circ$ angular step for perfectly smooth anti-aliased arcs with $< 0.2\text{ ms}$ render time.
- **Hardware Mirroring (HUD)**:
  - `setHudMode(bool hud)` writes register `0x36` (`MADCTL`, `0x02` for X-axis flip, `0x00` for normal) directly to `bus` without manipulating vendor files.

### 2.2 `AmoledHWPolicy` (`src/Device_T_Display_S3_AMOLED/Policies/AmoledHWPolicy.h`)
- **Encapsulation**:
  - All anonymous namespace globals are removed.
  - Touch controller instance (`TouchDrvCST92xx touch`) and gesture tracking state (`startX`, `startY`, `currentX`, `currentY`, `swiping`, `lastPoll`) are encapsulated as private static members of `AmoledHWPolicy`.
- **Named Geometric & Timing Constants**:
  ```cpp
  static constexpr int16_t TOUCH_WIDTH           = 466;
  static constexpr int16_t TOUCH_HEIGHT          = 466;
  static constexpr int16_t TAP_ZONE_BOTTOM_Y     = 350;
  static constexpr int16_t TAP_ZONE_SPLIT_X      = TOUCH_WIDTH / 2; // 233
  static constexpr int16_t SWIPE_MIN_DISTANCE    = 30;
  static constexpr uint32_t TOUCH_POLL_PERIOD_MS = 15; // 66 Hz
  ```
- **Gesture Finite State Machine**:
  - **Touch Down**: Captures starting coordinates with horizontal inversion: `rx = TOUCH_WIDTH - touchX[0]`.
  - **Touch Move**: Updates current coordinates while `swiping == true`.
  - **Touch Up**: 
    - If $\max(|\Delta X|, |\Delta Y|) > \text{SWIPE\_MIN\_DISTANCE} \implies$ Horizontal swipe pushes `HW_NAV_DELTA` ($\pm 1$), Vertical swipe pushes `HW_ACTION_TOGGLE`.
    - Else (Tap) $\implies$ $Y > 350$ pushes `HW_ACTION_TOGGLE`, $X < 233$ pushes `HW_NAV_DELTA -1` (decrement), $X \ge 233$ pushes `HW_NAV_DELTA +1` (increment).
- **Power & Pin Management**:
  - `initBoard()` coordinates display/touch rail enable (GPIO 38), I2C Fast Mode (`Wire.setClock(400000); Wire.setTimeOut(50);`), `TP_INT` pull-up, and ADC battery pin.

### 2.3 `main.cpp` (`src/Device_T_Display_S3_AMOLED/main.cpp`)
- FreeRTOS Task Separation:
  - `inputTask`: Core 0, Priority 5, 10ms delay (200Hz polling rate).
  - `uiTask`: Core 1, Priority 1 (offscreen rendering & QSPI flush).
  - `logicTask`: Core 0, Priority 1, 50ms delay (BLE / state machine).

---

## 3. Verification Plan
1. **Compilation**: `platformio run -e T_Display_S3_AMOLED` completes with `SUCCESS` and 0 warnings.
2. **Native Unity Unit Tests**: `platformio test -e unit_tests` passes 100%.
3. **Hardware Runtime**:
   - Touch taps on `+` increment values; taps on `-` decrement values.
   - Horizontal swipes switch screens cleanly.
   - Arc gauge redraws instantly with atomic double buffering and no clockwise animation.
   - Serial monitor contains zero I2C NACK / read error logs.
