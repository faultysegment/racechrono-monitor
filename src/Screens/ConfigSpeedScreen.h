#pragma once
#include "IScreen.h"
#include <cstdio>

template <typename DisplayPolicy>
class ConfigSpeedScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, Model& model) override {
        tft.fillScreen(0x0000); 
        tft.setTextColor(0xFFFF, 0x0000); 
        tft.setTextSize(2);
        
        int screenW = tft.width();
        tft.setCursor(screenW / 2 - 66, 10);
        tft.print("SPEED LIMIT");
        
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
        snprintf(buf, sizeof(buf), "%.1f  ", model.speedLimit);
        tft.print(buf);
    }
};
