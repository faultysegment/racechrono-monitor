#pragma once

#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_mac.h>
#include <Wire.h>
#define XPOWERS_CHIP_BQ25896
#include <XPowersLib.h>
#include <RotaryEncoder.h>
#include <lvgl.h>
#include "../../EventBus.h"

class RealHWPolicy {
    RotaryEncoder* encoder = nullptr;
    PowersBQ25896 pmu;

    lv_indev_drv_t indev_drv;
    lv_indev_t* enc_indev = nullptr;
    int lastIndevDelta = 0;

    static void encIsr(void* arg) {
        auto* self = static_cast<RealHWPolicy*>(arg);
        if (self && self->encoder) {
            self->encoder->tick();
        }
    }

    static void encoder_read(lv_indev_drv_t* drv, lv_indev_data_t* data) {
        auto* self = static_cast<RealHWPolicy*>(drv->user_data);
        if (!self) return;
        data->enc_diff = self->lastIndevDelta;
        self->lastIndevDelta = 0;
        if (self->isActionKeyPressed()) {
            data->state = LV_INDEV_STATE_PR;
        } else {
            data->state = LV_INDEV_STATE_REL;
        }
    }

public:
    RealHWPolicy() = default;
    ~RealHWPolicy() {
        if (encoder) {
            delete encoder;
            encoder = nullptr;
        }
    }

    void initBoard() {
        // T-Embed specific pins
        ::pinMode(46, OUTPUT);
        ::digitalWrite(46, HIGH);

        ::pinMode(15, OUTPUT); // BOARD_PWR_EN
        ::digitalWrite(15, HIGH);
        
        ::pinMode(6, INPUT_PULLUP); // BOARD_USER_KEY
        
        ::delay(100);

        ::pinMode(39, OUTPUT); ::digitalWrite(39, HIGH); 
        ::pinMode(41, OUTPUT); ::digitalWrite(41, HIGH); 
        ::pinMode(13, OUTPUT); ::digitalWrite(13, HIGH); 
        
        ::delay(100);

        ::pinMode(0, INPUT_PULLUP); // ENCODER_KEY
        
        if (!encoder) {
            encoder = new RotaryEncoder(4, 5, RotaryEncoder::LatchMode::TWO03);
            ::attachInterruptArg(digitalPinToInterrupt(4), encIsr, this, CHANGE);
            ::attachInterruptArg(digitalPinToInterrupt(5), encIsr, this, CHANGE);
        }
    }

    void initIndev() {
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_ENCODER;
        indev_drv.read_cb = encoder_read;
        indev_drv.user_data = this;
        enc_indev = lv_indev_drv_register(&indev_drv);
    }
    
    int getNavigationDelta() {
        if (!encoder) return 0;
        encoder->tick();
        int dir = (int)encoder->getDirection();
        lastIndevDelta += dir;
        return dir;
    }

    void pollExtraEvents(EventBus& bus) {}

    bool isPowerKeyPressed() {
        return ::digitalRead(6) == LOW; // BOARD_USER_KEY
    }

    bool isActionKeyPressed() {
        return ::digitalRead(0) == LOW; // ENCODER_KEY
    }

    void powerOffBoard() {
        ::digitalWrite(15, LOW); // BOARD_PWR_EN
        // Wait until the user releases the button, otherwise it immediately wakes up from deep sleep
        while (::digitalRead(6) == LOW) {
            ::delay(50);
        }
        esp_sleep_enable_ext0_wakeup((gpio_num_t)6, LOW); // BOARD_USER_KEY
        esp_deep_sleep_start();
    }

    void reboot() {
        ESP.restart();
    }

    void initBattery() {
        Wire.begin(8, 18);
        if (pmu.init(Wire, 8, 18, BQ25896_SLAVE_ADDRESS)) {
            pmu.enableMeasure();
        }
    }
    
    int getBatteryPercent() {
        uint16_t vbat = pmu.getBattVoltage();
        if (vbat == 0) return -1;
        int pct = (vbat - 3200) / 10;
        if (pct < 0) pct = 0;
        if (pct > 100) pct = 100;
        return pct;
    }

    void delay(uint32_t ms) {
        ::delay(ms);
    }
    
    uint32_t millis() {
        return ::millis();
    }
    
    void getMacDefault(uint8_t* mac) {
        esp_efuse_mac_get_default(mac);
    }
};
