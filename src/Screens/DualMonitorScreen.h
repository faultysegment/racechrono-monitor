#pragma once
#include "IScreen.h"
#include "../UI.h"
#include <cmath>
#include <cstdio>
#include <cstring>

template <typename DisplayPolicy>
class DualMonitorScreen : public IScreen<DisplayPolicy> {
    ScreenSlotConfig mTopSlot;
    ScreenSlotConfig mBtmSlot;

public:
    DualMonitorScreen(int topIdx = 0, int btmIdx = 1) : mTopSlot(topIdx), mBtmSlot(btmIdx) {}
    DualMonitorScreen(const ScreenSlotConfig& top, const ScreenSlotConfig& btm) : mTopSlot(top), mBtmSlot(btm) {}

    void setSlots(const ScreenSlotConfig& top, const ScreenSlotConfig& btm) {
        mTopSlot = top;
        mBtmSlot = btm;
    }

    void setMonitors(int topIdx, int btmIdx) {
        mTopSlot.monitorIndex = topIdx;
        mBtmSlot.monitorIndex = btmIdx;
    }

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        UI<DisplayPolicy> ui(tft);
        ui.begin();

        float barH = 0.23f; 
        
        // Draw Top Monitor
        if (mTopSlot.monitorIndex >= 0 && state.nextMonitorId > mTopSlot.monitorIndex) {
            drawMonitor(ui, state, mTopSlot, 0.05f, 0.23f, barH);
        }
        
        // Draw Bottom Monitor
        if (mBtmSlot.monitorIndex >= 0 && state.nextMonitorId > mBtmSlot.monitorIndex) {
            drawMonitor(ui, state, mBtmSlot, 0.52f, 0.70f, barH);
        }
    }

private:
    void drawMonitor(UI<DisplayPolicy>& ui, AppState& state, const ScreenSlotConfig& slot, float textY, float barY, float barH) {
        int mIdx = slot.monitorIndex;
        float* limitPtr = state.monitors[mIdx].limitPtr;
        float currentLimit = limitPtr ? *limitPtr : 1.0f;
        
        char prefix = (state.monitors[mIdx].title[0] != '\0') ? state.monitors[mIdx].title[0] : ' ';

        if (state.monitors[mIdx].hasException) {
            char valBuf[32];
            snprintf(valBuf, sizeof(valBuf), "%c   ERR   ", prefix);
            
            ui.setCursorY(textY);
            ui.textCenter(valBuf, 0xF800, 0.15f, -0.05f); // RED
            
            ui.setCursorY(barY);
            ui.emptyBar(0x7BEF, barH);
        } else if (state.monitors[mIdx].value != AppState::INVALID_VALUE) {
            float val = (float)state.monitors[mIdx].value * state.monitors[mIdx].multiplier;
            
            uint32_t color = 0x7BEF; // DARKGREY
            if (val > 0) {
                color = slot.positiveColor;
            } else if (val < 0) {
                color = slot.negativeColor;
            }
            
            char valBuf[32];
            if (state.monitors[mIdx].decimals == 2) {
                snprintf(valBuf, sizeof(valBuf), "%c  %+.2f   ", prefix, val);
            } else {
                snprintf(valBuf, sizeof(valBuf), "%c  %+.1f   ", prefix, val);
            }
            
            ui.setCursorY(textY);
            ui.textCenter(valBuf, color, 0.15f, -0.05f); // nudge left slightly more for dual
            
            ui.setCursorY(barY);
            ui.bar(val, currentLimit, color, 0x7BEF, barH);
            
        } else {
            char valBuf[32];
            if (state.monitors[mIdx].decimals == 2) {
                snprintf(valBuf, sizeof(valBuf), "%c  --.--   ", prefix);
            } else {
                snprintf(valBuf, sizeof(valBuf), "%c  --.-   ", prefix);
            }
            
            ui.setCursorY(textY);
            ui.textCenter(valBuf, 0x7BEF, 0.15f, -0.05f);
            
            ui.setCursorY(barY);
            ui.emptyBar(0x7BEF, barH);
        }
    }
};
