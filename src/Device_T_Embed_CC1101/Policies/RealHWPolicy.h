#pragma once

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
    static void initBoard() {
        // T-Embed specific pins
        ::pinMode(46, OUTPUT);
        ::digitalWrite(46, HIGH);

        ::pinMode(15, OUTPUT); // BOARD_PWR_EN
        ::digitalWrite(15, HIGH);
        
        ::pinMode(6, INPUT_PULLUP); // BOARD_USER_KEY
        
        ::delay(100);

        ::pinMode(39, OUTPUT); ::digitalWrite(39, HIGH); 
        ::pinMode(41, OUTPUT); ::digitalWrite(41, HIGH); 
        
        ::delay(100);

        ::pinMode(0, INPUT_PULLUP); // ENCODER_KEY
        
        if (!encoder) {
            encoder = new RotaryEncoder(4, 5, RotaryEncoder::LatchMode::TWO03);
            ::attachInterrupt(digitalPinToInterrupt(4), encIsr, CHANGE);
            ::attachInterrupt(digitalPinToInterrupt(5), encIsr, CHANGE);
        }
    }
    
    static int getNavigationDelta() {
        if (!encoder) return 0;
        encoder->tick();
        int dir = (int)encoder->getDirection();
        return dir;
    }

    static bool isPowerKeyPressed() {
        return ::digitalRead(6) == LOW; // BOARD_USER_KEY
    }

    static bool isActionKeyPressed() {
        return ::digitalRead(0) == LOW; // ENCODER_KEY
    }

    static void powerOffBoard() {
        ::digitalWrite(15, LOW); // BOARD_PWR_EN
        // Wait until the user releases the button, otherwise it immediately wakes up from deep sleep
        while (::digitalRead(6) == LOW) {
            ::delay(50);
        }
        esp_sleep_enable_ext0_wakeup((gpio_num_t)6, LOW); // BOARD_USER_KEY
        esp_deep_sleep_start();
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
