#pragma once
#include <cstdint>

struct MockHWPolicy {
    static uint32_t currentMillis;
    static int buttonState;
    static int encoderKeyState;
    static bool sleepCalled;

    static int encoderDelta;
    static int batteryPercent;

    static void reset() {
        currentMillis = 0;
        buttonState = 1; // HIGH usually
        encoderKeyState = 1; // HIGH usually
        sleepCalled = false;
        encoderDelta = 0;
        batteryPercent = 100;
    }

    static void initEncoder(uint8_t pinA, uint8_t pinB) {}
    
    static int getEncoderDelta() {
        int d = encoderDelta;
        encoderDelta = 0;
        return d;
    }

    static void initBattery() {}
    static int getBatteryPercent() { return batteryPercent; }

    static void pinMode(uint8_t pin, uint8_t mode) {}
    static void digitalWrite(uint8_t pin, uint8_t val) {}
    static int digitalRead(uint8_t pin) {
        if (pin == 6) return buttonState; // BOARD_USER_KEY
        if (pin == 0) return encoderKeyState; // ENCODER_KEY
        return 0;
    }
    
    static void delay(uint32_t ms) {
        currentMillis += ms;
    }
    
    static uint32_t millis() {
        return currentMillis;
    }
    
    static void sleepEnableExt0Wakeup(uint8_t pin, int level) {}
    
    static void deepSleepStart() {
        sleepCalled = true;
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
int MockHWPolicy::buttonState = 1;
int MockHWPolicy::encoderKeyState = 1;
bool MockHWPolicy::sleepCalled = false;
int MockHWPolicy::encoderDelta = 0;
int MockHWPolicy::batteryPercent = 100;
#endif
