#pragma once
#include "IScreen.h"
#include "../CircularUI.h"
#include <cmath>
#include <cstdio>

template <typename DisplayPolicy>
class CircularDualMonitorScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        CircularUI<DisplayPolicy> ui(tft);
        
        if (state.nextMonitorId < 2) {
            ui.textCenter("WAIT", 0xFFFF, 0.12f, 0.5f);
            return;
        }

        // Top monitor (Index 0)
        float currentLimit0 = state.monitors[0].limitPtr ? *state.monitors[0].limitPtr : 1.0f;
        float val0 = (state.monitors[0].value != AppState::INVALID_VALUE) ? (float)state.monitors[0].value * state.monitors[0].multiplier : 0.0f;
        
        // Bottom monitor (Index 1)
        float currentLimit1 = state.monitors[1].limitPtr ? *state.monitors[1].limitPtr : 1.0f;
        float val1 = (state.monitors[1].value != AppState::INVALID_VALUE) ? (float)state.monitors[1].value * state.monitors[1].multiplier : 0.0f;

        // Clear center
        int cx = tft.width() / 2;
        int cy = tft.height() / 2;
        int r = std::min(cx, cy) - ui.h(0.12f);
        tft.fillCircle(cx, cy, r, 0x0000);

        char buf0[32], buf1[32];
        snprintf(buf0, sizeof(buf0), "%.1f", val0);
        snprintf(buf1, sizeof(buf1), "%.1f", val1);
        
        ui.textCenter(state.monitors[0].title, 0x7BEF, 0.06f, 0.20f);
        ui.textCenter(buf0, 0xFFFF, 0.12f, 0.33f);
        
        ui.textCenter(buf1, 0xFFFF, 0.12f, 0.67f);
        ui.textCenter(state.monitors[1].title, 0x7BEF, 0.06f, 0.80f);
        
        // Draw top arc for monitor 0 (0 to 180 degrees roughly)
        float angle0 = (std::abs(val0) / currentLimit0) * 180.0f;
        if (angle0 > 180.0f) angle0 = 180.0f;
        
        int radiusOut = std::min(cx, cy) - 5;
        int radiusIn = radiusOut - ui.h(0.1f);
        
        tft.fillArc(cx, cy, radiusOut, radiusIn, 270.0f, 270.0f + 180.0f, 0x7BEF); // Top half background
        if (angle0 > 0) tft.fillArc(cx, cy, radiusOut, radiusIn, 270.0f, 270.0f + angle0, 0x07E0); // Top half foreground
        
        // Draw bottom arc for monitor 1 (180 to 360 degrees)
        float angle1 = (std::abs(val1) / currentLimit1) * 180.0f;
        if (angle1 > 180.0f) angle1 = 180.0f;
        
        tft.fillArc(cx, cy, radiusOut, radiusIn, 90.0f, 90.0f + 180.0f, 0x7BEF); // Bottom half background
        if (angle1 > 0) tft.fillArc(cx, cy, radiusOut, radiusIn, 90.0f, 90.0f + angle1, 0x07E0); // Bottom half foreground
    }
};
