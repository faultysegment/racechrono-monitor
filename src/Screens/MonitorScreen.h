#pragma once
#include "IScreen.h"
#include "../UI.h"
#include <cmath>
#include <cstdio>

template <typename DisplayPolicy>
class MonitorScreen : public IScreen<DisplayPolicy> {
    int mIdx;
public:
    MonitorScreen(int monitorIndex) : mIdx(monitorIndex) {}

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        UI<DisplayPolicy> ui(tft);
        ui.begin();
        ui.space(0.05f); // 5% spacing
        if (state.nextMonitorId > mIdx) {
            ui.textLeft(state.monitors[mIdx].title, 0xFFFF, 0.15f, 0.05f);
        } else {
            ui.textLeft("WAIT", 0xFFFF, 0.15f, 0.05f);
        }
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        if (state.nextMonitorId > mIdx) {
            UI<DisplayPolicy> ui(tft);
            ui.begin();
            
            float* limitPtr = state.monitors[mIdx].limitPtr;
            float currentLimit = limitPtr ? *limitPtr : 1.0f;
            
            if (state.isEditMode) {
                ui.setCursorY(0.25f);
                char buf[32];
                snprintf(buf, sizeof(buf), " LIM: %.1f  ", currentLimit);
                ui.textCenter(buf, 0xFFE0, 0.20f);
                
                ui.setCursorY(0.70f);
                ui.emptyBar(0x7BEF, 0.25f);
            } else {
                if (state.monitors[mIdx].hasException) {
                    ui.setCursorY(0.25f);
                    ui.textCenter("   ERR   ", 0xF800, 0.20f);
                    
                    ui.setCursorY(0.70f);
                    ui.emptyBar(0x7BEF, 0.25f);
                } else if (state.monitors[mIdx].value != AppState::INVALID_VALUE) {
                    float val = (float)state.monitors[mIdx].value * state.monitors[mIdx].multiplier;
                    
                    uint32_t color = 0x7BEF; // DARKGREY
                    if (val > 0) {
                        color = state.monitors[mIdx].positiveIsGood ? 0x07E0 : 0xF800; // GREEN : RED
                    } else if (val < 0) {
                        color = state.monitors[mIdx].positiveIsGood ? 0xF800 : 0x07E0; // RED : GREEN
                    }
                    
                    char valBuf[32];
                    if (state.monitors[mIdx].decimals == 2) {
                        snprintf(valBuf, sizeof(valBuf), "   %+.2f   ", val);
                    } else {
                        snprintf(valBuf, sizeof(valBuf), "   %+.1f   ", val);
                    }
                    
                    ui.setCursorY(0.25f);
                    ui.textCenter(valBuf, color, 0.20f);
                    
                    ui.setCursorY(0.70f);
                    ui.bar(val, currentLimit, color, 0x7BEF, 0.25f);
                } else {
                    ui.setCursorY(0.25f);
                    if (state.monitors[mIdx].decimals == 2) {
                        ui.textCenter("   --.--   ", 0x7BEF, 0.20f);
                    } else {
                        ui.textCenter("   --.-   ", 0x7BEF, 0.20f);
                    }
                    
                    ui.setCursorY(0.70f);
                    ui.emptyBar(0x7BEF, 0.25f);
                }
            }
        }
    }
};
