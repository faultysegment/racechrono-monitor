#pragma once
#include "IScreen.h"
#include <cmath>
#include <cstdio>
#include <cstring>

template <typename DisplayPolicy>
class DualMonitorScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        static bool lastEditMode = false;
        if (lastEditMode != state.isEditMode) {
            tft.fillScreen(0x0000);
            lastEditMode = state.isEditMode;
        }

        int screenW = tft.width();
        int screenH = tft.height();

        if (state.isEditMode) {
            tft.setTextColor(0xFFE0, 0x0000);
            tft.setTextSize(2);
            tft.setCursor(screenW / 2 - 120, screenH / 2 - 10);
            tft.print("EDIT ON SINGLE SCREEN");
            return;
        }

        int barH = 40; 
        
        // Draw Monitor 0 (Top)
        if (state.nextMonitorId > 0) {
            drawMonitor(tft, state, 0, 0, 40, screenW, barH, 10);
        }
        
        // Draw Monitor 1 (Bottom)
        if (state.nextMonitorId > 1) {
            drawMonitor(tft, state, 1, 0, 120, screenW, barH, 90);
        }
    }

private:
    void drawMonitor(DisplayPolicy& tft, AppState& state, int mIdx, int barX, int barY, int maxBarW, int barH, int textY) {
        float* limitPtr = state.monitors[mIdx].limitPtr;
        float currentLimit = limitPtr ? *limitPtr : 1.0f;
        
        char prefix = ' ';
        uint32_t goodColor = 0x07E0; // GREEN
        uint32_t badColor = 0xF800;  // RED

        if (strcmp(state.monitors[mIdx].title, "TIME") == 0) {
            goodColor = 0x07E0;
            badColor = 0xF800;
            prefix = 'T';
        } else if (strcmp(state.monitors[mIdx].title, "SPEED") == 0) {
            goodColor = 0x07FF; // CYAN
            badColor = 0xFD20;  // ORANGE
            prefix = 'S';
        }

        if (state.monitors[mIdx].value != AppState::INVALID_VALUE) {
            float val = (float)state.monitors[mIdx].value * state.monitors[mIdx].multiplier;
            
            uint32_t color = 0x7BEF; // DARKGREY
            if (val > 0) {
                color = state.monitors[mIdx].positiveIsGood ? goodColor : badColor;
            } else if (val < 0) {
                color = state.monitors[mIdx].positiveIsGood ? badColor : goodColor;
            }
            
            float absVal = std::abs(val);
            if (absVal > currentLimit) absVal = currentLimit;
            
            int fillW = (int)((absVal / currentLimit) * maxBarW);
            
            tft.fillRect(barX, barY, fillW, barH, color); 
            tft.fillRect(barX + fillW, barY, maxBarW - fillW, barH, 0x7BEF); 
            
            // Draw numeric value
            tft.setTextColor(color, 0x0000);
            tft.setTextSize(3);
            char valBuf[32];
            
            if (state.monitors[mIdx].decimals == 2) {
                snprintf(valBuf, sizeof(valBuf), "%c  %+.2f   ", prefix, val);
            } else {
                snprintf(valBuf, sizeof(valBuf), "%c  %+.1f   ", prefix, val);
            }
            
            tft.setCursor(maxBarW / 2 - 100, textY);
            tft.print(valBuf);
            
        } else {
            tft.fillRect(barX, barY, maxBarW, barH, 0x7BEF);
            tft.setTextColor(0x7BEF, 0x0000);
            tft.setTextSize(3);
            tft.setCursor(maxBarW / 2 - 100, textY);
            char valBuf[32];
            if (state.monitors[mIdx].decimals == 2) {
                snprintf(valBuf, sizeof(valBuf), "%c  --.--   ", prefix);
            } else {
                snprintf(valBuf, sizeof(valBuf), "%c  --.-   ", prefix);
            }
            tft.print(valBuf);
        }
    }
};



