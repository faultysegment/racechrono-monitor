#pragma once
#include <cstdint>
#include "../../EventBus.h"

struct MockHWPolicy {
    uint32_t currentMillis = 0;
    bool powerKeyPressed = false;
    bool actionKeyPressed = false;
    bool sleepCalled = false;
    bool rebootCalled = false;

    int navigationDelta = 0;
    int batteryPercent = 100;

    void reset() {
        currentMillis = 0;
        powerKeyPressed = false;
        actionKeyPressed = false;
        sleepCalled = false;
        rebootCalled = false;
        navigationDelta = 0;
        batteryPercent = 100;
    }

    void initBoard() {}
    
    int getNavigationDelta() {
        int d = navigationDelta;
        navigationDelta = 0;
        return d;
    }

    void pollExtraEvents(EventBus& bus) {}

    bool isPowerKeyPressed() {
        return powerKeyPressed;
    }

    bool isActionKeyPressed() {
        return actionKeyPressed;
    }

    void powerOffBoard() {
        sleepCalled = true;
    }

    void reboot() {
        rebootCalled = true;
    }

    void initBattery() {}
    int getBatteryPercent() { return batteryPercent; }

    void delay(uint32_t ms) {
        currentMillis += ms;
    }
    
    uint32_t millis() {
        return currentMillis;
    }
    
    void getMacDefault(uint8_t* mac) {
        mac[0] = 0xAA; mac[1] = 0xBB; mac[2] = 0xCC; 
        mac[3] = 0xDD; mac[4] = 0xEE; mac[5] = 0xFF;
    }
};
