#pragma once
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdlib.h>

class RealHWPolicy {
public:
    static void initBoard() {}
    static void initBattery() {}
    
    static void powerOffBoard() {
        exit(0);
    }
    
    static int getBatteryPercent() { 
        return 100; 
    }
    
    static uint32_t millis() {
        return SDL_GetTicks();
    }
    
    static void delay(uint32_t ms) {
        SDL_Delay(ms);
    }

    static bool isPowerKeyPressed() {
        SDL_PumpEvents();
        const Uint8* state = SDL_GetKeyboardState(NULL);
        return state[SDL_SCANCODE_ESCAPE];
    }

    static bool isActionKeyPressed() {
        SDL_PumpEvents();
        const Uint8* state = SDL_GetKeyboardState(NULL);
        return state[SDL_SCANCODE_SPACE];
    }

    static int getNavigationDelta() {
        SDL_PumpEvents();
        const Uint8* state = SDL_GetKeyboardState(NULL);
        static bool leftPressed = false;
        static bool rightPressed = false;
        
        int delta = 0;
        if (state[SDL_SCANCODE_LEFT] && !leftPressed) {
            delta = -1;
            leftPressed = true;
        } else if (!state[SDL_SCANCODE_LEFT]) {
            leftPressed = false;
        }

        if (state[SDL_SCANCODE_RIGHT] && !rightPressed) {
            delta = 1;
            rightPressed = true;
        } else if (!state[SDL_SCANCODE_RIGHT]) {
            rightPressed = false;
        }
        
        return delta;
    }

    static void getMacDefault(uint8_t* mac) {
        mac[0] = 0; mac[1] = 0; mac[2] = 0; mac[3] = 0; mac[4] = 0xAA; mac[5] = 0xBB;
    }
};
