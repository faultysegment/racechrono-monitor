#pragma once

#include <Arduino.h>
#include <esp_sleep.h>
#include <Wire.h>
#include "pin_config.h"
#include "../../EventBus.h"
#include <TouchDrvCST92xx.h>

class AmoledHWPolicy {
    static constexpr int16_t TOUCH_WIDTH           = 466;
    static constexpr int16_t TOUCH_HEIGHT          = 466;
    static constexpr int16_t TAP_ZONE_BOTTOM_Y     = 350;
    static constexpr int16_t TAP_ZONE_SPLIT_X      = TOUCH_WIDTH / 2;
    static constexpr int16_t SWIPE_MIN_DISTANCE    = 30;
    static constexpr uint32_t TOUCH_POLL_PERIOD_MS = 20;

    TouchDrvCST92xx touch;
    int16_t touchX[5] = {0};
    int16_t touchY[5] = {0};
    int16_t startX = 0;
    int16_t startY = 0;
    int16_t currentX = 0;
    int16_t currentY = 0;
    bool isSwiping = false;
    uint32_t lastPoll = 0;

    void finishGesture(EventBus& bus) {
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
    AmoledHWPolicy() = default;

    void initBoard() {
        pinMode(38, OUTPUT);
        digitalWrite(38, HIGH);

        Wire.begin(IIC_SDA, IIC_SCL);
        touch.jumpCheck();
        touch.setPins(-1, TP_INT);
        pinMode(TP_INT, INPUT_PULLUP);
        touch.begin(Wire, 0x5A, IIC_SDA, IIC_SCL);
    }

    void pollExtraEvents(EventBus& bus) {
        uint32_t now = ::millis();
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

    int getNavigationDelta() { return 0; }
    bool isPowerKeyPressed() { return digitalRead(0) == LOW; }
    bool isActionKeyPressed() { return false; }

    void powerOffBoard() {
        while (digitalRead(0) == LOW) delay(50);
        esp_sleep_enable_ext0_wakeup((gpio_num_t)0, LOW);
        esp_deep_sleep_start();
    }

    void reboot() {
        ESP.restart();
    }

    void initBattery() {}
    int getBatteryPercent() { return 100; }

    void delay(uint32_t ms) { ::delay(ms); }
    uint32_t millis() { return ::millis(); }
    void getMacDefault(uint8_t* mac) { esp_efuse_mac_get_default(mac); }
};
