#pragma once
#include "IScreen.h"
#include "../CircularUI.h"
#include <cmath>
#include <cstdio>

template <typename DisplayPolicy>
class CircularConfigMonitorScreen : public IScreen<DisplayPolicy> {
    int mIdx;
public:
    CircularConfigMonitorScreen(int monitorIndex) : mIdx(monitorIndex) {}

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        CircularUI<DisplayPolicy> ui(tft);
        tft.fillScreen(0x0000); 

        const char* title = (mIdx == 1) ? "Speed Limit" : "Time Limit";
        float currentLimit = (mIdx == 1) ? state.speedLimit : state.timeLimit;

        if (state.isEditMode) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.1f", currentLimit);
            
            ui.textCenter("EDIT", 0xFFFF, 0.08f, 0.23f);
            ui.textCenter(buf, 0xFFE0, 0.18f, 0.50f);
            
            ui.drawPlusMinus(0xFFFF);
            ui.drawCheckmark(0x07E0);
            
            ui.circularBar(1.0f, 1.0f, 0x7BEF, 0x7BEF, 0.05f);
            
        } else {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.1f", currentLimit);
            
            ui.textCenter(title, 0xFFFF, 0.08f, 0.23f);
            ui.textCenter(buf, 0x7BEF, 0.18f, 0.50f);
            
            ui.circularBar(0, currentLimit, 0x0000, 0x7BEF, 0.1f);
        }
    }
};
