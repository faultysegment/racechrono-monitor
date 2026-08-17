# HUD Mirrored Screens Design Specification

## Goal & Background
The user plans to use the device screen as a Heads-Up Display (HUD) projected onto a car's windshield. When reflected off the glass, standard screen displays appear horizontally flipped (reversed). 

To support HUD usage without duplicating screen UI layout logic, this feature adds horizontally mirrored (HUD) versions of all existing screens into the screen carousel cycle across all supported hardware targets (T-Embed CC1101, T-Display S3 AMOLED) and the SDL2 native simulator.

## Architectural Design

### 1. Decorator Component (`HudScreenWrapper<DisplayPolicy>`)
A new template decorator class `HudScreenWrapper` implementing `IScreen<DisplayPolicy>` is introduced in `src/Screens/HudScreenWrapper.h`.

- Wraps an underlying `IScreen<DisplayPolicy>* innerScreen`.
- In `onShow(tft, state)` and `onUpdate(tft, state)`:
  - Invokes `tft.setHudMode(true)` on the display policy.
  - Delegates execution to `innerScreen->onShow(tft, state)` or `innerScreen->onUpdate(tft, state)`.
- Core `View<DisplayPolicy, HWPolicy>::update()` resets display HUD mode to `false` (`tft.setHudMode(false)`) before drawing standard screens.

### 2. Display Policy Interface Extensions
Every `DisplayPolicy` class (`RealDisplayPolicy`, `AmoledDisplayPolicy`, `MockDisplayPolicy`) implements `setHudMode(bool enabled)`:

- **`Device_T_Embed_CC1101/Policies/RealDisplayPolicy.h` (TFT_eSPI)**:
  - Standard mode: `setRotation(3)` (Landscape).
  - HUD mode: `setRotation(7)` (Landscape Mirrored via hardware register in TFT_eSPI).
- **`Device_T_Display_S3_AMOLED/Policies/AmoledDisplayPolicy.h` (Arduino_GFX)**:
  - Standard mode: `setRotation(1)`.
  - HUD mode: `setRotation(5)` / mirrored rotation mode in Arduino_GFX.
- **`Device_Native/Policies/RealDisplayPolicy.h` (SDL2 Simulator)**:
  - Tracks `isHudMode`.
  - In `flush()`, renders the frame target texture to renderer using `SDL_RenderCopyEx` with `SDL_FLIP_HORIZONTAL` when `isHudMode` is true.
- **`Device_Mock/Policies/MockDisplayPolicy.h` (Unit Tests)**:
  - Stores `bool hudMode` state for verification assertions.

### 3. Screen Registration & Carousel Order
In `TEmbedViewPolicy`, `AmoledViewPolicy`, and `NativeViewPolicy`, screens are registered in `appView` grouped as: standard screens first, followed by HUD screens.

**Connected Screens**:
1. `monitor0` (Standard Monitor 0)
2. `monitor1` (Standard Monitor 1)
3. `dualMonitor` (Standard Dual Monitor)
4. `hudMonitor0` (`HudScreenWrapper` wrapping `monitor0`)
5. `hudMonitor1` (`HudScreenWrapper` wrapping `monitor1`)
6. `hudDualMonitor` (`HudScreenWrapper` wrapping `dualMonitor`)

**Disconnected Screens**:
1. `disconnectedMsg` (Standard Disconnected Message)
2. `configSpeed` (Standard Speed Limit Config)
3. `configTime` (Standard Time Limit Config)
4. `hudDisconnectedMsg` (`HudScreenWrapper` wrapping `disconnectedMsg`)
5. `hudConfigSpeed` (`HudScreenWrapper` wrapping `configSpeed`)
6. `hudConfigTime` (`HudScreenWrapper` wrapping `configTime`)

## Testing & Verification Plan

### Automated Unit Tests (`pio test -e unit_tests`)
- Verify `HudScreenWrapper` toggles `setHudMode(true)` on update.
- Verify total screen count and index navigation in `AppState` and `View`.

### Native Simulator (`pio run -e run_simulator`)
- Interactive verification in SDL2 simulator, verifying horizontal flipping of text, gauge arcs, and UI bounds when scrolling into HUD screen section.
