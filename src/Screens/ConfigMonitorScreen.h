#pragma once
#include "IScreen.h"
#include "../UI.h"
#include <cstdio>

template <typename DisplayPolicy>
class ConfigMonitorScreen : public IScreen<DisplayPolicy> {
    const char* title;
    float* limitPtr;
public:
    ConfigMonitorScreen(const char* title, float* limitPtr) : title(title), limitPtr(limitPtr) {}

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        UI<DisplayPolicy> ui(tft);
        ui.begin();
        ui.setCursorY(0.1f);
        ui.textCenter(title, 0xFFFF, 0.15f);
        
        drawLimit(ui, state);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        UI<DisplayPolicy> ui(tft);
        ui.begin();
        drawLimit(ui, state);
    }

private:
    void drawLimit(UI<DisplayPolicy>& ui, AppState& state) {
        ui.setCursorY(0.4f);
        uint32_t color = state.isEditMode ? 0xFFE0 : 0xFFFF; // Yellow if editing
        
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f  ", *limitPtr);
        ui.textCenter(buf, color, 0.25f);
    }
};
