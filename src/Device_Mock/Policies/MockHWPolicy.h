#pragma once
#include <cstdint>

struct MockHWPolicy {
    static uint32_t currentMillis;
    static bool powerKeyPressed;
    static bool actionKeyPressed;
    static bool sleepCalled;

    static int navigationDelta;
    static int batteryPercent;

    static void reset() {
        currentMillis = 0;
        powerKeyPressed = false;
        actionKeyPressed = false;
        sleepCalled = false;
        navigationDelta = 0;
        batteryPercent = 100;
    }

    static void initBoard() {}
    
    static int getNavigationDelta() {
        int d = navigationDelta;
        navigationDelta = 0;
        return d;
    }

    static bool isPowerKeyPressed() {
        return powerKeyPressed;
    }

    static bool isActionKeyPressed() {
        return actionKeyPressed;
    }

    static void powerOffBoard() {
        sleepCalled = true;
    }

    static void initBattery() {}
    static int getBatteryPercent() { return batteryPercent; }

    static void delay(uint32_t ms) {
        currentMillis += ms;
    }
    
    static uint32_t millis() {
        return currentMillis;
    }
    
    static void getMacDefault(uint8_t* mac) {
        mac[0] = 0xAA; mac[1] = 0xBB; mac[2] = 0xCC; 
        mac[3] = 0xDD; mac[4] = 0xEE; mac[5] = 0xFF;
    }
};

// These should normally be in a .cpp file to avoid multiple definition errors
// but for unity tests we usually compile them per-test executable.
#ifdef PIO_UNIT_TESTING
uint32_t MockHWPolicy::currentMillis = 0;
bool MockHWPolicy::powerKeyPressed = false;
bool MockHWPolicy::actionKeyPressed = false;
bool MockHWPolicy::sleepCalled = false;
int MockHWPolicy::navigationDelta = 0;
int MockHWPolicy::batteryPercent = 100;
#endif
