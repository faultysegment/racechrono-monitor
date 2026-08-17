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

    static TouchDrvCST92xx& getTouch() { static TouchDrvCST92xx t; return t; }
    static int16_t* getTouchX() { static int16_t x[5] = {0}; return x; }
    static int16_t* getTouchY() { static int16_t y[5] = {0}; return y; }
    static int16_t& getStartX() { static int16_t sx = 0; return sx; }
    static int16_t& getStartY() { static int16_t sy = 0; return sy; }
    static int16_t& getCurrentX() { static int16_t cx = 0; return cx; }
    static int16_t& getCurrentY() { static int16_t cy = 0; return cy; }
    static bool& getIsSwiping() { static bool s = false; return s; }

    static void finishGesture(EventBus& bus) {
        bool& isSwiping = getIsSwiping();
        if (!isSwiping) return;
        isSwiping = false;

        int dx = getCurrentX() - getStartX();
        int dy = getCurrentY() - getStartY();

        if (abs(dx) > SWIPE_MIN_DISTANCE || abs(dy) > SWIPE_MIN_DISTANCE) {
            if (abs(dx) > abs(dy)) {
                int delta = (dx > 0) ? -1 : 1;
                bus.push(Event{EventType::HW_NAV_DELTA, delta, 0, 0});
            } else {
                bus.push(Event{EventType::HW_ACTION_TOGGLE, 0, 0, 0});
            }
        } else {
            if (getStartY() > TAP_ZONE_BOTTOM_Y) {
                bus.push(Event{EventType::HW_ACTION_TOGGLE, 0, 0, 0});
            } else if (getStartX() < TAP_ZONE_SPLIT_X) {
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

        TouchDrvCST92xx& touch = getTouch();
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

        bool& isSwiping = getIsSwiping();
        if (digitalRead(TP_INT) == LOW || isSwiping) {
            TouchDrvCST92xx& touch = getTouch();
            int16_t* touchX = getTouchX();
            int16_t* touchY = getTouchY();
            uint8_t touched = touch.getPoint(touchX, touchY, 1);
            if (touched > 0) {
                int16_t rx = TOUCH_WIDTH - touchX[0];
                int16_t ry = touchY[0];

                if (!isSwiping) {
                    getStartX() = rx;
                    getStartY() = ry;
                    isSwiping = true;
                }
                getCurrentX() = rx;
                getCurrentY() = ry;
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
