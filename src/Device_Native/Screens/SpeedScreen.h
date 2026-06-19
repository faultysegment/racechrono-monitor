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
            int barH = 40;
            int barY = screenH - barH - 10;
            int barX = 0;
            int maxBarW = screenW;
            
            if (model.isEditMode) {
                // In edit mode, hide metrics, show only limit
                tft.setTextColor(0xFFE0, 0x0000);
                tft.setTextSize(3);
                char buf[32];
                snprintf(buf, sizeof(buf), " LIM: %.1f  ", model.speedLimit);
                tft.setCursor(screenW / 2 - 100, 50);
                tft.print(buf);
                
                // Draw a blank grey bar
                tft.fillRect(barX, barY, maxBarW, barH, 0x7BEF);
            } else {
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
                    
                    // Draw numeric value
                    tft.setTextColor(color, 0x0000);
                    tft.setTextSize(3);
                    char valBuf[32];
                    snprintf(valBuf, sizeof(valBuf), "   %+.1f   ", val);
                    tft.setCursor(screenW / 2 - 100, 50);
                    tft.print(valBuf);
                    
                } else {
                    tft.fillRect(barX, barY, maxBarW, barH, 0x7BEF);
                    tft.setTextColor(0x7BEF, 0x0000);
                    tft.setTextSize(3);
                    tft.setCursor(screenW / 2 - 100, 50);
                    tft.print("   --.-   ");
                }
            }
        }
    }
};
