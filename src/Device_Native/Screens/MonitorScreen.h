#pragma once
#include "IScreen.h"
#include <cmath>
#include <cstdio>

template <typename DisplayPolicy>
class MonitorScreen : public IScreen<DisplayPolicy> {
    int mIdx;
public:
    MonitorScreen(int monitorIndex) : mIdx(monitorIndex) {}

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        tft.setTextColor(0xFFFF, 0x0000); 
        tft.setTextSize(6);
        tft.setCursor(30, 30);
        if (state.nextMonitorId > mIdx) {
            tft.print(state.monitors[mIdx].title);
        } else {
            tft.print("WAIT");
        }
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        if (state.nextMonitorId > mIdx) {
            int screenW = tft.width();
            int screenH = tft.height();
            int barH = 150;
            int barY = screenH - barH - 40;
            int barX = 0;
            int maxBarW = screenW;
            
            float* limitPtr = state.monitors[mIdx].limitPtr;
            float currentLimit = limitPtr ? *limitPtr : 1.0f;
            
            if (state.isEditMode) {
                // In edit mode, hide metrics, show only limit
                tft.setTextColor(0xFFE0, 0x0000);
                tft.setTextSize(10);
                char buf[32];
                snprintf(buf, sizeof(buf), " LIM: %.1f  ", currentLimit);
                tft.setCursor(screenW / 2 - 400, screenH / 2 - 200);
                tft.print(buf);
                
                // Draw a blank grey bar
                tft.fillRect(barX, barY, maxBarW, barH, 0x7BEF);
            } else {
                if (state.monitors[mIdx].value != AppState::INVALID_VALUE) {
                    float val = (float)state.monitors[mIdx].value * state.monitors[mIdx].multiplier;
                    
                    uint32_t color = 0x7BEF; // DARKGREY
                    if (val > 0) {
                        color = state.monitors[mIdx].positiveIsGood ? 0x07E0 : 0xF800; // GREEN : RED
                    } else if (val < 0) {
                        color = state.monitors[mIdx].positiveIsGood ? 0xF800 : 0x07E0; // RED : GREEN
                    }
                    
                    float absVal = std::abs(val);
                    if (absVal > currentLimit) absVal = currentLimit;
                    
                    int fillW = (int)((absVal / currentLimit) * maxBarW);
                    
                    tft.fillRect(barX, barY, fillW, barH, color); 
                    tft.fillRect(barX + fillW, barY, maxBarW - fillW, barH, 0x7BEF); 
                    
                    // Draw numeric value
                    tft.setTextColor(color, 0x0000);
                    tft.setTextSize(10);
                    char valBuf[32];
                    
                    // Format based on decimals
                    if (state.monitors[mIdx].decimals == 2) {
                        snprintf(valBuf, sizeof(valBuf), "   %+.2f   ", val);
                    } else {
                        snprintf(valBuf, sizeof(valBuf), "   %+.1f   ", val);
                    }
                    
                    tft.setCursor(screenW / 2 - 400, screenH / 2 - 200);
                    tft.print(valBuf);
                    
                } else {
                    tft.fillRect(barX, barY, maxBarW, barH, 0x7BEF);
                    tft.setTextColor(0x7BEF, 0x0000);
                    tft.setTextSize(10);
                    tft.setCursor(screenW / 2 - 400, screenH / 2 - 200);
                    if (state.monitors[mIdx].decimals == 2) {
                        tft.print("   --.--   ");
                    } else {
                        tft.print("   --.-   ");
                    }
                }
            }
        }
    }
};



