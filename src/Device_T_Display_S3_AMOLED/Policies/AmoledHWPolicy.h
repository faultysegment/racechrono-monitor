#pragma once

#include <Arduino.h>
#include <esp_sleep.h>
#include <Wire.h>
#include "pin_config.h"
#include "../../EventBus.h"

// If using 1.75 inch, use CST9217 driver.
// If using 1.43 inch, use FT3168 (not handled here fully, but assuming 1.75 inch for now).
#include <TouchDrvCST92xx.h>

namespace {
    TouchDrvCST92xx touch;
    int16_t touchX[5], touchY[5];
    volatile bool isTouchPressed = false;
    uint32_t lastTouchMillis = 0;
    
    // Swipe & Tap detection vars
    int16_t startX = 0, startY = 0;
    int16_t currentX = 0, currentY = 0;
    bool swiping = false;

    void IRAM_ATTR touchIsr() {
        isTouchPressed = true;
    }

    void finishGesture(EventBus& bus) {
        if (!swiping) return;
        swiping = false;
        
        uint32_t dur = ::millis() - lastTouchMillis;
        int dx = currentX - startX;
        int dy = currentY - startY;

        if (dur < 1000) { // Valid gesture within 1s
            if (abs(dx) > 40 || abs(dy) > 40) {
                // Swipe gesture
                if (abs(dx) > abs(dy)) {
                    int delta = (dx > 0) ? -1 : 1; // Left swipe = next screen, Right swipe = prev screen
                    bus.push(Event{EventType::HW_NAV_DELTA, delta, 0, 0});
                } else {
                    // Vertical Swipe -> mode toggle
                    bus.push(Event{EventType::HW_ACTION_TOGGLE, 0, 0, 0});
                }
            } else {
                // Tap gesture
                if (startY > 350) { 
                    // Tap bottom area -> Action Toggle
                    bus.push(Event{EventType::HW_ACTION_TOGGLE, 0, 0, 0});
                } else if (startX < 200) {
                    // Tap left area -> Previous screen
                    bus.push(Event{EventType::HW_NAV_DELTA, -1, 0, 0});
                } else if (startX >= 200) {
                    // Tap right area -> Next screen
                    bus.push(Event{EventType::HW_NAV_DELTA, 1, 0, 0});
                }
            }
        }
    }
}

struct AmoledHWPolicy {
    static void initBoard() {
        // Enable power for Display and Touch (IO38)
        pinMode(38, OUTPUT);
        digitalWrite(38, HIGH);
        delay(50); // wait for power to stabilize

        // Init I2C
        Wire.begin(IIC_SDA, IIC_SCL);

        // Init Touch
        touch.jumpCheck();
        touch.setPins(-1, TP_INT);
        if (touch.begin(Wire, 0x5A, IIC_SDA, IIC_SCL)) {
            pinMode(TP_INT, INPUT_PULLUP);
            attachInterrupt(TP_INT, touchIsr, FALLING);
        }

        // Init battery ADC pin
        pinMode(BATTERY_VOLTAGE_ADC_DATA, INPUT);
    }
    
    static void pollExtraEvents(EventBus& bus) {
        if (isTouchPressed || swiping || digitalRead(TP_INT) == LOW) {
            isTouchPressed = false;
            uint8_t touched = touch.getPoint(touchX, touchY, touch.getSupportTouchPoint());
            if (touched > 0) {
                if (!swiping) {
                    startX = touchX[0];
                    startY = touchY[0];
                    swiping = true;
                    lastTouchMillis = ::millis();
                }
                currentX = touchX[0];
                currentY = touchY[0];
            } else if (swiping) {
                finishGesture(bus);
            }
        }
    }

    static int getNavigationDelta() {
        return 0; // Handled via pollExtraEvents
    }

    static bool isPowerKeyPressed() {
        // BOOT button is GPIO 0
        return digitalRead(0) == LOW;
    }

    static bool isActionKeyPressed() {
        return false; // Handled via pollExtraEvents
    }

    static void powerOffBoard() {
        // Wait until button released
        while (digitalRead(0) == LOW) {
            delay(50);
        }
        esp_sleep_enable_ext0_wakeup((gpio_num_t)0, LOW);
        esp_deep_sleep_start();
    }

    static void initBattery() {
        // Already init in initBoard if needed
    }
    
    static int getBatteryPercent() {
        // Read ADC
        int adc = analogRead(BATTERY_VOLTAGE_ADC_DATA);
        // Map voltage (approximation, assuming voltage divider)
        // Adjust these values based on actual battery curve
        float voltage = (adc / 4095.0) * 3.3 * 2; // Example assuming 1:1 divider
        int pct = (int)((voltage - 3.2) / (4.2 - 3.2) * 100);
        if (pct < 0) pct = 0;
        if (pct > 100) pct = 100;
        return pct;
    }

    static void delay(uint32_t ms) {
        ::delay(ms);
    }
    
    static uint32_t millis() {
        return ::millis();
    }
    
    static void getMacDefault(uint8_t* mac) {
        esp_efuse_mac_get_default(mac);
    }
};
