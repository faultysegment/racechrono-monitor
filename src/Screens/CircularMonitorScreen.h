#pragma once
#include "IScreen.h"
#include "../CircularUI.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

template <typename DisplayPolicy>
class CircularMonitorScreen : public IScreen<DisplayPolicy> {
    int mIdx;
    float lastVal;
    uint32_t lastColor;
    bool forceFullRedraw;

public:
    CircularMonitorScreen(int monitorIndex) 
        : mIdx(monitorIndex), lastVal(-9999.0f), lastColor(0), forceFullRedraw(true) {}

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        lastVal = -9999.0f;
        lastColor = 0;
        forceFullRedraw = true;
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        CircularUI<DisplayPolicy> ui(tft);
        
        if (state.nextMonitorId <= mIdx) {
            tft.fillScreen(0x0000);
            ui.circularRadialBar(0, 1.0f, 0x0000, 0x7BEF);
            ui.textCenter("WAIT", 0xFFFF, 0.15f, 0.5f);
            return;
        }

        float* limitPtr = state.monitors[mIdx].limitPtr;
        float currentLimit = limitPtr ? *limitPtr : 1.0f;

        if (state.monitors[mIdx].hasException) {
            tft.fillScreen(0x0000);
            ui.circularRadialBar(0, currentLimit, 0x0000, 0x7BEF);
            ui.textCenter(state.monitors[mIdx].title, 0x001F, 0.1f, 0.25f);
            ui.textCenter("ERR", 0xF800, 0.3f, 0.5f);
            return;
        }

        if (state.monitors[mIdx].value != AppState::INVALID_VALUE) {
            float val = (float)state.monitors[mIdx].value * state.monitors[mIdx].multiplier;
            
            uint32_t color = 0x7BEF;
            if (val > 0) {
                color = state.monitors[mIdx].positiveIsGood ? 0x07E0 : 0xF800;
            } else if (val < 0) {
                color = state.monitors[mIdx].positiveIsGood ? 0xF800 : 0x07E0;
            }

            if (forceFullRedraw || color != lastColor || std::abs(val - lastVal) >= 0.01f) {
                forceFullRedraw = false;
                lastColor = color;
                lastVal = val;

                // Draw radial bar first - clears inner black center automatically
                ui.circularRadialBar(val, currentLimit, color, 0x7BEF);

                char valBuf[32];
                if (state.monitors[mIdx].decimals == 2) {
                    snprintf(valBuf, sizeof(valBuf), "%+.2f", val);
                } else {
                    snprintf(valBuf, sizeof(valBuf), "%+.1f", val);
                }

                int cx = tft.width() / 2;
                int cy = tft.height() / 2;
                int radiusOut = std::min(cx, cy) - 5;
                float absVal = std::abs(val);
                if (absVal > currentLimit) absVal = currentLimit;
                float pct = (currentLimit > 0.0001f) ? (absVal / currentLimit) : 0.0f;
                int rIn = (int)std::round((float)radiusOut * (1.0f - pct));

                // Adjust contrast when radial bar closes into the center
                uint32_t titleColor = (rIn < ui.h(0.35f)) ? 0x0000 : 0x001F;

                ui.textCenter(state.monitors[mIdx].title, titleColor, 0.1f, 0.25f);
                ui.textCenter(valBuf, 0x001F, 0.25f, 0.5f);
            }
        } else {
            tft.fillScreen(0x0000);
            ui.circularRadialBar(0, currentLimit, 0x0000, 0x7BEF);
            ui.textCenter(state.monitors[mIdx].title, 0x001F, 0.1f, 0.25f);
            ui.textCenter("---", 0xFFFF, 0.3f, 0.5f);
        }
    }
};
