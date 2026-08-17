# AMOLED Subsystem Refactoring Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Refactor the LilyGO T-Display S3 AMOLED device layer into a clean, encapsulated, robust architecture without global anonymous state or runtime canvas checks.

**Architecture:** Encapsulate PSRAM double-buffered display pipeline in `AmoledDisplayPolicy` with zero-overhead direct dispatch; encapsulate CST9217 touch gesture detection in `AmoledHWPolicy` using named geometric constants and clean FSM; decouple multi-core FreeRTOS tasks cleanly in `main.cpp`.

**Tech Stack:** ESP32-S3, Arduino-ESP32, FreeRTOS, Arduino_GFX, CST9217, PlatformIO.

**Spec:** [docs/superpowers/specs/2026-08-17-amoled-refactoring-design.md](file:///c:/Users/faultysegment/Documents/PlatformIO/Projects/racechrono-monitor/docs/superpowers/specs/2026-08-17-amoled-refactoring-design.md)

## Global Constraints
- Target platform: ESP32-S3 (`T_Display_S3_AMOLED` environment).
- Driver libraries in `lib_T_Display_S3_AMOLED/` must remain unmodified.
- Zero `#ifdef` spaghetti in core files (`AppLogic`, `View`, `AppState`).
- Unit tests (`env:unit_tests`) must pass at all times.

---

### Task 1: Refactor `AmoledDisplayPolicy.h`

**Files:**
- Modify: `src/Device_T_Display_S3_AMOLED/Policies/AmoledDisplayPolicy.h`

**Interfaces:**
- Consumes: `pin_config.h`, `Arduino_GFX_Library.h`
- Produces: `AmoledDisplayPolicy` class with clean drawing interface and private static `bus`, `gfx`, `canvas` pointers.

- [ ] **Step 1: Write encapsulated `AmoledDisplayPolicy`**

```cpp
#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"

class AmoledDisplayPolicy {
private:
    static inline Arduino_DataBus* bus = nullptr;
    static inline Arduino_GFX* gfx = nullptr;
    static inline Arduino_Canvas* canvas = nullptr;
    static inline bool isHudMode = false;

public:
    void init() {
        if (!bus) {
            bus = new Arduino_ESP32QSPI(
                LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
        }
        if (!gfx) {
#if defined DO0143FAT01
            gfx = new Arduino_SH8601(bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT);
#elif (defined DO0143FMST10) || (defined H0175Y003AM)
            gfx = new Arduino_CO5300(bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT, 6, 0, 0, 0);
#endif
        }

#if defined(LCD_EN)
        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, HIGH);
        delay(50);
#endif

        gfx->begin();

        if (!canvas) {
            canvas = new Arduino_Canvas(LCD_WIDTH, LCD_HEIGHT, gfx);
            canvas->begin(GFX_SKIP_OUTPUT_BEGIN);
        }
    }

    void setRotation(uint8_t r) { if (canvas) canvas->setRotation(r); }

    void setHudMode(bool hud) {
        if (isHudMode != hud) {
            isHudMode = hud;
            if (bus) {
                bus->beginWrite();
                bus->writeC8D8(0x36, hud ? 0x02 : 0x00);
                bus->endWrite();
            }
        }
    }

    void fillScreen(uint32_t color) { canvas->fillScreen(color); }
    void setCursor(int16_t x, int16_t y) { canvas->setCursor(x, y); }
    void setTextWrap(bool wrap) { canvas->setTextWrap(wrap); }
    void setTextSize(uint8_t size) { canvas->setTextSize(size); }
    void setTextColor(uint32_t c, uint32_t bg) { canvas->setTextColor(c, bg); }
    void print(const char* str) { canvas->print(str); }
    void print(int n) { canvas->print(n); }
    void println(const char* str) { canvas->println(str); }

    int16_t textWidth(const char* str) {
        int16_t x1, y1;
        uint16_t w, h;
        canvas->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
        return w;
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) { canvas->fillRect(x, y, w, h, color); }
    void fillCircle(int16_t x, int16_t y, int16_t r, uint32_t color) { canvas->fillCircle(x, y, r, color); }
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint32_t color) { canvas->drawFastHLine(x, y, w, color); }
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint32_t color) { canvas->drawFastVLine(x, y, h, color); }

    void fillArc(int16_t cx, int16_t cy, int16_t r1, int16_t r2, float startAngle, float endAngle, uint16_t color) {
        if (r1 == r2) return;
        if (r1 < r2) { int16_t tmp = r1; r1 = r2; r2 = tmp; }

        while (endAngle < startAngle) endAngle += 360.0f;
        float sweep = endAngle - startAngle;
        if (sweep <= 0.001f) return;
        if (sweep > 360.0f) sweep = 360.0f;

        constexpr float STEP = 2.0f;
        int numSteps = (int)(sweep / STEP);
        if (numSteps < 1) numSteps = 1;
        float actualStep = sweep / numSteps;

        constexpr float DEG2RAD = 3.14159265358979323846f / 180.0f;

        float rad0 = startAngle * DEG2RAD;
        float sin0 = sinf(rad0);
        float cos0 = cosf(rad0);

        int16_t x0_out = cx + (int16_t)roundf(r1 * sin0);
        int16_t y0_out = cy - (int16_t)roundf(r1 * cos0);
        int16_t x0_in  = cx + (int16_t)roundf(r2 * sin0);
        int16_t y0_in  = cy - (int16_t)roundf(r2 * cos0);

        for (int i = 1; i <= numSteps; i++) {
            float a = startAngle + i * actualStep;
            float rad1 = a * DEG2RAD;
            float sin1 = sinf(rad1);
            float cos1 = cosf(rad1);

            int16_t x1_out = cx + (int16_t)roundf(r1 * sin1);
            int16_t y1_out = cy - (int16_t)roundf(r1 * cos1);
            int16_t x1_in  = cx + (int16_t)roundf(r2 * sin1);
            int16_t y1_in  = cy - (int16_t)roundf(r2 * cos1);

            canvas->fillTriangle(x0_out, y0_out, x1_out, y1_out, x0_in, y0_in, color);
            canvas->fillTriangle(x0_in, y0_in, x1_out, y1_out, x1_in, y1_in, color);

            x0_out = x1_out;
            y0_out = y1_out;
            x0_in  = x1_in;
            y0_in  = y1_in;
        }
    }

    int16_t width() { return canvas ? canvas->width() : LCD_WIDTH; }
    int16_t height() { return canvas ? canvas->height() : LCD_HEIGHT; }

    void flush() {
        if (canvas) canvas->flush();
    }

    void setBacklight(bool on) {
#if defined(LCD_EN)
        digitalWrite(LCD_EN, on ? HIGH : LOW);
#endif
        if (gfx) gfx->Display_Brightness(on ? 255 : 0);
    }

    void drawBattery(int percent, bool force = false) {
        static int lastBat = -2;
        if (force || lastBat != percent) {
            lastBat = percent;
            int screenW = width();
            setTextSize(2);
            setTextColor(0xFFFF, 0x0000);
            setCursor(screenW - 55, 10);
            char buf[16];
            if (percent >= 0 && percent <= 100) {
                snprintf(buf, sizeof(buf), "%3d%%", percent);
            } else {
                snprintf(buf, sizeof(buf), "---%%");
            }
            print(buf);
        }
    }
};
```

- [ ] **Step 2: Verify compilation**

Run: `& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e T_Display_S3_AMOLED`
Expected: SUCCESS

- [ ] **Step 3: Commit**

```bash
git add src/Device_T_Display_S3_AMOLED/Policies/AmoledDisplayPolicy.h
git commit -m "refactor(amoled): encapsulate bus/gfx/canvas and eliminate redundant runtime checks"
```

---

### Task 2: Refactor `AmoledHWPolicy.h`

**Files:**
- Modify: `src/Device_T_Display_S3_AMOLED/Policies/AmoledHWPolicy.h`

**Interfaces:**
- Consumes: `pin_config.h`, `TouchDrvCST92xx.h`, `EventBus.h`
- Produces: `AmoledHWPolicy` struct with private static touch/gesture state and named constants.

- [ ] **Step 1: Write encapsulated `AmoledHWPolicy`**

```cpp
#pragma once

#include <Arduino.h>
#include <esp_sleep.h>
#include <Wire.h>
#include "pin_config.h"
#include "../../EventBus.h"
#include <TouchDrvCST92xx.h>

struct AmoledHWPolicy {
private:
    static constexpr int16_t TOUCH_WIDTH           = 466;
    static constexpr int16_t TOUCH_HEIGHT          = 466;
    static constexpr int16_t TAP_ZONE_BOTTOM_Y     = 350;
    static constexpr int16_t TAP_ZONE_SPLIT_X      = TOUCH_WIDTH / 2;
    static constexpr int16_t SWIPE_MIN_DISTANCE    = 30;
    static constexpr uint32_t TOUCH_POLL_PERIOD_MS = 15;

    static inline TouchDrvCST92xx touch;
    static inline int16_t touchX[5] = {0}, touchY[5] = {0};
    static inline int16_t startX = 0, startY = 0;
    static inline int16_t currentX = 0, currentY = 0;
    static inline bool isSwiping = false;

    static void finishGesture(EventBus& bus) {
        if (!isSwiping) return;
        isSwiping = false;

        int dx = currentX - startX;
        int dy = currentY - startY;

        if (abs(dx) > SWIPE_MIN_DISTANCE || abs(dy) > SWIPE_MIN_DISTANCE) {
            if (abs(dx) > abs(dy)) {
                int delta = (dx > 0) ? -1 : 1;
                bus.push(Event{EventType::HW_NAV_DELTA, delta, 0, 0});
            } else {
                bus.push(Event{EventType::HW_ACTION_TOGGLE, 0, 0, 0});
            }
        } else {
            if (startY > TAP_ZONE_BOTTOM_Y) {
                bus.push(Event{EventType::HW_ACTION_TOGGLE, 0, 0, 0});
            } else if (startX < TAP_ZONE_SPLIT_X) {
                bus.push(Event{EventType::HW_NAV_DELTA, -1, 0, 0});
            } else {
                bus.push(Event{EventType::HW_NAV_DELTA, 1, 0, 0});
            }
        }
    }

public:
    static void initBoard() {
        pinMode(38, OUTPUT);
        digitalWrite(38, HIGH);
        delay(50);

        Wire.begin(IIC_SDA, IIC_SCL);
        Wire.setClock(400000);
        Wire.setTimeOut(50);

        touch.jumpCheck();
        touch.setPins(-1, TP_INT);
        pinMode(TP_INT, INPUT_PULLUP);
        touch.begin(Wire, 0x5A, IIC_SDA, IIC_SCL);

        pinMode(BATTERY_VOLTAGE_ADC_DATA, INPUT);
    }

    static void pollExtraEvents(EventBus& bus) {
        uint32_t now = ::millis();
        static uint32_t lastPoll = 0;
        if (now - lastPoll < TOUCH_POLL_PERIOD_MS) return;
        lastPoll = now;

        if (digitalRead(TP_INT) == LOW || isSwiping) {
            uint8_t touched = touch.getPoint(touchX, touchY, 1);
            if (touched > 0) {
                int16_t rx = TOUCH_WIDTH - touchX[0];
                int16_t ry = touchY[0];

                if (!isSwiping) {
                    startX = rx;
                    startY = ry;
                    isSwiping = true;
                }
                currentX = rx;
                currentY = ry;
            } else if (isSwiping) {
                finishGesture(bus);
            }
        }
    }

    static int getNavigationDelta() { return 0; }
    static bool isPowerKeyPressed() { return digitalRead(0) == LOW; }
    static bool isActionKeyPressed() { return false; }

    static void powerOffBoard() {
        while (digitalRead(0) == LOW) delay(50);
        esp_sleep_enable_ext0_wakeup((gpio_num_t)0, LOW);
        esp_deep_sleep_start();
    }

    static void initBattery() {}

    static int getBatteryPercent() {
        int adc = analogRead(BATTERY_VOLTAGE_ADC_DATA);
        float voltage = (adc / 4095.0) * 3.3 * 2;
        int pct = (int)((voltage - 3.2) / (4.2 - 3.2) * 100);
        if (pct < 0) pct = 0;
        if (pct > 100) pct = 100;
        return pct;
    }

    static void delay(uint32_t ms) { ::delay(ms); }
    static uint32_t millis() { return ::millis(); }
    static void getMacDefault(uint8_t* mac) { esp_efuse_mac_get_default(mac); }
};
```

- [ ] **Step 2: Verify compilation**

Run: `& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e T_Display_S3_AMOLED`
Expected: SUCCESS

- [ ] **Step 3: Commit**

```bash
git add src/Device_T_Display_S3_AMOLED/Policies/AmoledHWPolicy.h
git commit -m "refactor(amoled): encapsulate touch state in AmoledHWPolicy with named geometry constants"
```

---

### Task 3: Clean up `main.cpp` and Verify All Environments

**Files:**
- Modify: `src/Device_T_Display_S3_AMOLED/main.cpp`

**Interfaces:**
- Consumes: `App.h`, `AmoledDisplayPolicy.h`, `AmoledHWPolicy.h`, `AmoledBLEPolicy.h`, `AmoledStoragePolicy.h`, `AmoledViewPolicy.h`
- Produces: Clean entry point without global raw display pointers.

- [ ] **Step 1: Refactor `main.cpp`**

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <BLEDevice.h>
#include <Preferences.h>
#include <Arduino_GFX_Library.h>

#include "Policies/AmoledDisplayPolicy.h"
#include "Policies/AmoledHWPolicy.h"
#include "Policies/AmoledBLEPolicy.h"
#include "Policies/AmoledStoragePolicy.h"
#include "../../App.h"
#include "Policies/AmoledViewPolicy.h"

App<AmoledDisplayPolicy, AmoledHWPolicy, AmoledBLEPolicy, AmoledStoragePolicy, AmoledViewPolicy<AmoledDisplayPolicy>> app;

void uiTask(void* pvParameters) {
    Event e;
    while (1) {
        if (app.getEventBus().pop_with_timeout(e, 25)) {
            app.processEvent(e);
        } else {
            app.tickUI();
        }
    }
}

void inputTask(void* pvParameters) {
    while (1) {
        app.pollInput();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void logicTask(void* pvParameters) {
    while (1) {
        app.pollLogic();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup() {
    app.setup();

    // Pin Input to Core 0 (PRO_CPU) for dedicated high-priority touch polling
    xTaskCreatePinnedToCore(inputTask, "Input_Task", 4096, NULL, 5, NULL, 0);

    // Pin UI Rendering to Core 1 (APP_CPU) for dedicated display rendering
    xTaskCreatePinnedToCore(uiTask, "UI_Task", 4096, NULL, 1, NULL, 1);

    // Pin Logic to Core 0 (PRO_CPU)
    xTaskCreatePinnedToCore(logicTask, "Logic_Task", 4096, NULL, 1, NULL, 0);
}

void loop() {
    vTaskSuspend(NULL);
}
```

- [ ] **Step 2: Run Unit Tests**

Run: `& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" test -e unit_tests`
Expected: 14/14 tests PASSED

- [ ] **Step 3: Run Full Firmware Build**

Run: `& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e T_Display_S3_AMOLED`
Expected: SUCCESS

- [ ] **Step 4: Commit**

```bash
git add src/Device_T_Display_S3_AMOLED/main.cpp
git commit -m "refactor(amoled): clean up main.cpp entry point and remove obsolete global pointers"
```
