#pragma once
#include "IScreen.h"
#include <cmath>
#include <cstdio>
#include <cstring>

template <typename DisplayPolicy>
class DualMonitorScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, Model& model) override {
        tft.fillScreen(0x0000); 
    }

    void onUpdate(DisplayPolicy& tft, Model& model) override {
        static bool lastEditMode = false;
        if (lastEditMode != model.isEditMode) {
            tft.fillScreen(0x0000);
            lastEditMode = model.isEditMode;
        }

        int screenW = tft.width();
        int screenH = tft.height();

        if (model.isEditMode) {
            tft.setTextColor(0xFFE0, 0x0000);
            tft.setTextSize(5);
            tft.setCursor(screenW / 2 - 350, screenH / 2 - 50);
            tft.print("EDIT ON SINGLE SCREEN");
            return;
        }

        int barH = 120; // Thinner bars to fit two
        
        // Draw Monitor 0 (Top)
        if (model.nextMonitorId > 0) {
            drawMonitor(tft, model, 0, 0, 200, screenW, barH, 20);
        }
        
        // Draw Monitor 1 (Bottom)
        if (model.nextMonitorId > 1) {
            drawMonitor(tft, model, 1, 0, 560, screenW, barH, 380);
        }
    }

private:
    void drawMonitor(DisplayPolicy& tft, Model& model, int mIdx, int barX, int barY, int maxBarW, int barH, int textY) {
        float* limitPtr = model.monitors[mIdx].limitPtr;
        float currentLimit = limitPtr ? *limitPtr : 1.0f;
        
        char prefix = ' ';
        uint32_t goodColor = 0x07E0; // GREEN
        uint32_t badColor = 0xF800;  // RED

        if (strcmp(model.monitors[mIdx].title, "TIME") == 0) {
            goodColor = 0x07E0;
            badColor = 0xF800;
            prefix = 'T';
        } else if (strcmp(model.monitors[mIdx].title, "SPEED") == 0) {
            goodColor = 0x07FF; // CYAN
            badColor = 0xFD20;  // ORANGE
            prefix = 'S';
        }

        if (model.monitors[mIdx].value != Model::INVALID_VALUE) {
            float val = (float)model.monitors[mIdx].value * model.monitors[mIdx].multiplier;
            
            uint32_t color = 0x7BEF; // DARKGREY
            if (val > 0) {
                color = model.monitors[mIdx].positiveIsGood ? goodColor : badColor;
            } else if (val < 0) {
                color = model.monitors[mIdx].positiveIsGood ? badColor : goodColor;
            }
            
            float absVal = std::abs(val);
            if (absVal > currentLimit) absVal = currentLimit;
            
            int fillW = (int)((absVal / currentLimit) * maxBarW);
            
            tft.fillRect(barX, barY, fillW, barH, color); 
            tft.fillRect(barX + fillW, barY, maxBarW - fillW, barH, 0x7BEF); 
            
            // Draw numeric value
            tft.setTextColor(color, 0x0000);
            tft.setTextSize(8);
            char valBuf[32];
            
            if (model.monitors[mIdx].decimals == 2) {
                snprintf(valBuf, sizeof(valBuf), "%c  %+.2f   ", prefix, val);
            } else {
                snprintf(valBuf, sizeof(valBuf), "%c  %+.1f   ", prefix, val);
            }
            
            tft.setCursor(maxBarW / 2 - 400, textY);
            tft.print(valBuf);
            
        } else {
            tft.fillRect(barX, barY, maxBarW, barH, 0x7BEF);
            tft.setTextColor(0x7BEF, 0x0000);
            tft.setTextSize(8);
            tft.setCursor(maxBarW / 2 - 400, textY);
            char valBuf[32];
            if (model.monitors[mIdx].decimals == 2) {
                snprintf(valBuf, sizeof(valBuf), "%c  --.--   ", prefix);
            } else {
                snprintf(valBuf, sizeof(valBuf), "%c  --.-   ", prefix);
            }
            tft.print(valBuf);
        }
    }
};
