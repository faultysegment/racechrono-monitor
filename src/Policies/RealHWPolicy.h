#pragma once

#ifndef PIO_UNIT_TESTING
#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_mac.h>
#include <Wire.h>
#define XPOWERS_CHIP_BQ25896
#include <XPowersLib.h>
#include <RotaryEncoder.h>

namespace {
    RotaryEncoder* encoder = nullptr;
    PowersBQ25896 PMU;

    void IRAM_ATTR encIsr() {
        if (encoder) {
            encoder->tick();
        }
    }
}

struct RealHWPolicy {
    static void initEncoder(uint8_t pinA, uint8_t pinB) {
        if (!encoder) {
            encoder = new RotaryEncoder(pinA, pinB, RotaryEncoder::LatchMode::TWO03);
            ::attachInterrupt(digitalPinToInterrupt(pinA), encIsr, CHANGE);
            ::attachInterrupt(digitalPinToInterrupt(pinB), encIsr, CHANGE);
        }
    }
    
    static int getEncoderDelta() {
        if (!encoder) return 0;
        encoder->tick();
        int dir = (int)encoder->getDirection();
        return dir;
    }

    static void initBattery() {
        Wire.begin(8, 18);
        if (PMU.init(Wire, 8, 18, BQ25896_SLAVE_ADDRESS)) {
            PMU.enableMeasure();
        }
    }
    
    static int getBatteryPercent() {
        uint16_t vbat = PMU.getBattVoltage();
        if (vbat == 0) return -1;
        int pct = (vbat - 3200) / 10;
        if (pct < 0) pct = 0;
        if (pct > 100) pct = 100;
        return pct;
    }

    static void pinMode(uint8_t pin, uint8_t mode) {
        ::pinMode(pin, mode);
    }
    
    static void digitalWrite(uint8_t pin, uint8_t val) {
        ::digitalWrite(pin, val);
    }
    
    static int digitalRead(uint8_t pin) {
        return ::digitalRead(pin);
    }
    
    static void delay(uint32_t ms) {
        ::delay(ms);
    }
    
    static uint32_t millis() {
        return ::millis();
    }
    
    static void sleepEnableExt0Wakeup(uint8_t pin, int level) {
        esp_sleep_enable_ext0_wakeup((gpio_num_t)pin, level);
    }
    
    static void deepSleepStart() {
        esp_deep_sleep_start();
    }
    
    static void getMacDefault(uint8_t* mac) {
        esp_efuse_mac_get_default(mac);
    }
};
#endif
