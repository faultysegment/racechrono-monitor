#pragma once
#include "IScreen.h"
#include <cstdio>

template <typename DisplayPolicy>
class ConfigMonitorScreen : public IScreen<DisplayPolicy> {
    const char* title;
    float* limitPtr;
public:
    ConfigMonitorScreen(const char* title, float* limitPtr) : title(title), limitPtr(limitPtr) {}

    void onShow(DisplayPolicy& tft, Model& model) override {
        tft.fillScreen(0x0000); 
        tft.setTextColor(0xFFFF, 0x0000); 
        tft.setTextSize(2);
        
        int screenW = tft.width();
        tft.setCursor(screenW / 2 - 60, 10);
        tft.print(title);
        
        drawLimit(tft, model);
    }

    void onUpdate(DisplayPolicy& tft, Model& model) override {
        drawLimit(tft, model);
    }

private:
    void drawLimit(DisplayPolicy& tft, Model& model) {
        int screenW = tft.width();
        tft.setTextColor(model.isEditMode ? 0xFFE0 : 0xFFFF, 0x0000); // Yellow if editing
        tft.setCursor(screenW / 2 - 30, 40);
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f  ", *limitPtr);
        tft.print(buf);
    }
};
