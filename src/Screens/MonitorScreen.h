#pragma once
#include "IScreen.h"
#include "../UI.h"
#include <cmath>
#include <cstdio>

template <typename DisplayPolicy>
class MonitorScreen : public IScreen<DisplayPolicy> {
    ScreenSlotConfig mSlot;
public:
    MonitorScreen(int monitorIndex = 0) : mSlot(monitorIndex) {}
    MonitorScreen(const ScreenSlotConfig& slot) : mSlot(slot) {}

    void setConfig(const ScreenSlotConfig& slot) {
        mSlot = slot;
    }

    void setMonitorIndex(int idx) {
        mSlot.monitorIndex = idx;
    }

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        UI<DisplayPolicy> ui(tft);
        ui.begin();
        ui.space(0.05f); // 5% spacing
        if (state.nextMonitorId > mSlot.monitorIndex && mSlot.monitorIndex >= 0) {
            ui.textLeft(state.monitors[mSlot.monitorIndex].title, mSlot.titleColor, 0.15f, 0.05f);
        } else {
            ui.textLeft("WAIT", 0xFFFF, 0.15f, 0.05f);
        }
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        int mIdx = mSlot.monitorIndex;
        if (mIdx >= 0 && state.nextMonitorId > mIdx) {
            UI<DisplayPolicy> ui(tft);
            ui.begin();
            
            float* limitPtr = state.monitors[mIdx].limitPtr;
            float currentLimit = limitPtr ? *limitPtr : 1.0f;
            
            if (state.monitors[mIdx].hasException) {
                ui.setCursorY(0.25f);
                ui.textCenter("   ERR   ", 0xF800, 0.20f);
                
                ui.setCursorY(0.70f);
                ui.emptyBar(0x7BEF, 0.25f);
            } else if (state.monitors[mIdx].value != AppState::INVALID_VALUE) {
                float val = (float)state.monitors[mIdx].value * state.monitors[mIdx].multiplier;
                
                uint32_t color = 0x7BEF; // DARKGREY
                if (val > 0) {
                    color = mSlot.positiveColor;
                } else if (val < 0) {
                    color = mSlot.negativeColor;
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
};
