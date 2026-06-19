#pragma once
#include "IScreen.h"
#include <cmath>
#include <cstdio>

template <typename DisplayPolicy>
class SpeedScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, Model& model) override {
        tft.fillScreen(0x0000); 
        tft.setTextColor(0xFFFF, 0x0000); 
        tft.setTextSize(2);
        int screenW = tft.width();
        tft.setCursor(10, 10);
        tft.print("SPEED");
    }

    void onUpdate(DisplayPolicy& tft, Model& model) override {
        int mIdx = 1;
        if (model.nextMonitorId > mIdx) {
            int screenW = tft.width();
            int screenH = tft.height();
            int barH = screenH - 35;
            int barY = 35;
            int barX = 0;
            int maxBarW = screenW;
            
            if (model.monitors[mIdx].value != Model::INVALID_VALUE) {
                float val = (float)model.monitors[mIdx].value * model.monitors[mIdx].multiplier;
                uint32_t color = (val < 0) ? 0xF800 : 0x07E0; // RED : GREEN
                if (val == 0) color = 0x7BEF; // DARKGREY
                
                float absVal = std::abs(val);
                float maxVal = model.speedLimit;
                if (absVal > maxVal) absVal = maxVal;
                
                int fillW = (int)((absVal / maxVal) * maxBarW);
                
                tft.fillRect(barX, barY, fillW, barH, color); 
                tft.fillRect(barX + fillW, barY, maxBarW - fillW, barH, 0x7BEF); 
            } else {
                tft.fillRect(barX, barY, maxBarW, barH, 0x7BEF);
            }
            
            if (model.isEditMode) {
                tft.setTextColor(0xFFE0, 0x0000);
                tft.setCursor(80, 10);
                char buf[16];
                snprintf(buf, sizeof(buf), "LIM: %.1f ", model.speedLimit);
                tft.print(buf);
            }
        }
    }
};
