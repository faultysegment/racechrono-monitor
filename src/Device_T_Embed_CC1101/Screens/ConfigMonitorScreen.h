#pragma once
#include "IScreen.h"
#include <cstdio>

template <typename DisplayPolicy>
class ConfigMonitorScreen : public IScreen<DisplayPolicy> {
    const char* title;
    float* limitPtr;
public:
    ConfigMonitorScreen(const char* title, float* limitPtr) : title(title), limitPtr(limitPtr) {}

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        tft.setTextColor(0xFFFF, 0x0000); 
        tft.setTextSize(2);
        
        int screenW = tft.width();
        tft.setCursor(screenW / 2 - 60, 10);
        tft.print(title);
        
        drawLimit(tft, state);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        drawLimit(tft, state);
    }

private:
    void drawLimit(DisplayPolicy& tft, AppState& state) {
        int screenW = tft.width();
        tft.setTextColor(state.isEditMode ? 0xFFE0 : 0xFFFF, 0x0000); // Yellow if editing
        tft.setCursor(screenW / 2 - 30, 40);
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f  ", *limitPtr);
        tft.print(buf);
    }
};



