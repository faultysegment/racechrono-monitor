#pragma once
#include "BLEPolicyCallback.h"
#include <SDL2/SDL.h>
#include <string>

class RealBLEPolicy {
    BLEPolicyCallback* ctrl = nullptr;
    bool connected = false;
    bool indicating = false;
    uint32_t lastDataTime = 0;

    void sendDummyData() {
        if (!ctrl) return;
        
        // Send random dummy data so that visual value is between 0.1 and 0.2
        // Monitor 0 (Time) multiplier is 0.01 -> raw 10 to 20
        // Monitor 1 (Speed) multiplier is 0.036 -> raw 3 to 5 (0.108 to 0.180)
        int32_t timeVal = 10 + (rand() % 11); 
        int32_t speedVal = 3 + (rand() % 3);
        
        uint8_t rxData[10];
        
        // Monitor 0 (Time)
        rxData[0] = 0;
        rxData[1] = (timeVal >> 24) & 0xFF;
        rxData[2] = (timeVal >> 16) & 0xFF;
        rxData[3] = (timeVal >> 8) & 0xFF;
        rxData[4] = timeVal & 0xFF;

        // Monitor 1 (Speed)
        rxData[5] = 1;
        rxData[6] = (speedVal >> 24) & 0xFF;
        rxData[7] = (speedVal >> 16) & 0xFF;
        rxData[8] = (speedVal >> 8) & 0xFF;
        rxData[9] = speedVal & 0xFF;

        ctrl->onNotificationWrite(std::string((char*)rxData, 10));
    }

public:
    void init(const char* name, BLEPolicyCallback* controller) {
        ctrl = controller;
    }

    void startAdvertising() {}

    int getConnectedCount() {
        SDL_PumpEvents();
        const Uint8* state = SDL_GetKeyboardState(NULL);
        static bool enterPressed = false;

        if (state[SDL_SCANCODE_RETURN] && !enterPressed) {
            connected = !connected;
            enterPressed = true;
            if (!connected && ctrl) {
                ctrl->onBLEDisconnected();
            }
        } else if (!state[SDL_SCANCODE_RETURN]) {
            enterPressed = false;
        }

        if (connected) {
            uint32_t now = SDL_GetTicks();
            if (now - lastDataTime > 100) {
                sendDummyData();
                lastDataTime = now;
            }
        }

        return connected ? 1 : 0;
    }

    bool isConfigIndicating() {
        // Automatically indicate config is available if we connect
        return connected;
    }

    void indicateConfig(uint8_t* data, size_t len) {
        // Simulate immediate successful config write response
        if (ctrl) {
            uint8_t rx[2] = {0, (uint8_t)data[1]}; // CMD_RESULT_OK
            ctrl->onConfigWrite(std::string((char*)rx, 2));
        }
    }
};
