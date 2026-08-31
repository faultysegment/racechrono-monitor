#pragma once
#include "IScreen.h"
#include "../UI.h"
#include <cmath>
#include <cstdio>
#include <cstring>

template <typename DisplayPolicy>
class DualMonitorScreen : public IScreen<DisplayPolicy> {
    int mTopIdx;
    int mBtmIdx;

public:
    DualMonitorScreen(int topIdx = 0, int btmIdx = 1) : mTopIdx(topIdx), mBtmIdx(btmIdx) {}

    void setMonitors(int topIdx, int btmIdx) {
        mTopIdx = topIdx;
        mBtmIdx = btmIdx;
    }

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        UI<DisplayPolicy> ui(tft);
        ui.begin();

        float barH = 0.23f; 
        
        // Draw Top Monitor
        if (state.nextMonitorId > mTopIdx && mTopIdx >= 0) {
            drawMonitor(ui, state, mTopIdx, 0.05f, 0.23f, barH);
        }
        
        // Draw Bottom Monitor
        if (state.nextMonitorId > mBtmIdx && mBtmIdx >= 0) {
            drawMonitor(ui, state, mBtmIdx, 0.52f, 0.70f, barH);
        }
    }

private:
    void drawMonitor(UI<DisplayPolicy>& ui, AppState& state, int mIdx, float textY, float barY, float barH) {
        float* limitPtr = state.monitors[mIdx].limitPtr;
        float currentLimit = limitPtr ? *limitPtr : 1.0f;
        
        char prefix = (state.monitors[mIdx].title[0] != '\0') ? state.monitors[mIdx].title[0] : ' ';
        uint32_t goodColor = 0x07E0; // GREEN
        uint32_t badColor = 0xF800;  // RED

        if (strcmp(state.monitors[mIdx].title, "TIME") == 0) {
            goodColor = 0x07E0;
            badColor = 0xF800;
        } else if (strcmp(state.monitors[mIdx].title, "SPEED") == 0) {
            goodColor = 0x07FF; // CYAN
            badColor = 0xFD20;  // ORANGE
        }

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
                color = state.monitors[mIdx].positiveIsGood ? goodColor : badColor;
            } else if (val < 0) {
                color = state.monitors[mIdx].positiveIsGood ? badColor : goodColor;
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
