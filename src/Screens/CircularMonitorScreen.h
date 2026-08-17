#pragma once
#include "IScreen.h"
#include "../CircularUI.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

template <typename DisplayPolicy>
class CircularMonitorScreen : public IScreen<DisplayPolicy> {
    int mIdx;
    float lastAngle;
    float lastVal;
    uint32_t lastColor;
    bool forceFullRedraw;

public:
    CircularMonitorScreen(int monitorIndex) 
        : mIdx(monitorIndex), lastAngle(-1.0f), lastVal(-9999.0f), lastColor(0), forceFullRedraw(true) {}

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        lastAngle = -1.0f;
        lastVal = -9999.0f;
        lastColor = 0;
        forceFullRedraw = true;
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        CircularUI<DisplayPolicy> ui(tft);
        
        if (state.nextMonitorId <= mIdx) {
            ui.textCenter("WAIT", 0xFFFF, 0.12f, 0.5f);
            return;
        }

        float* limitPtr = state.monitors[mIdx].limitPtr;
        float currentLimit = limitPtr ? *limitPtr : 1.0f;

        if (state.isEditMode) {
            if (forceFullRedraw) {
                tft.fillScreen(0x0000);
                forceFullRedraw = false;
            }
            
            char buf[32];
            snprintf(buf, sizeof(buf), "%.1f", currentLimit);
            
            ui.textCenter("LIMIT", 0xFFFF, 0.08f, 0.23f);
            ui.textCenter(buf, 0xFFE0, 0.18f, 0.50f);
            
            ui.drawPlusMinus(0xFFFF);
            ui.drawCheckmark(0x07E0);
            
            ui.circularBar(1.0f, 1.0f, 0x7BEF, 0x7BEF, 0.05f);
            
        } else {
            if (state.monitors[mIdx].hasException) {
                tft.fillScreen(0x0000);
                ui.textCenter(state.monitors[mIdx].title, 0xFFFF, 0.08f, 0.23f);
                ui.textCenter("ERR", 0xF800, 0.18f, 0.50f);
                ui.circularBar(0, currentLimit, 0x0000, 0x7BEF, 0.1f);
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
                
                float absVal = std::abs(val);
                if (absVal > currentLimit) absVal = currentLimit;
                float angle = (absVal / currentLimit) * 360.0f;

                if (forceFullRedraw || color != lastColor) {
                    forceFullRedraw = false;
                    lastColor = color;
                    lastAngle = angle;
                    lastVal = val;

                    int cx = tft.width() / 2;
                    int cy = tft.height() / 2;
                    int r = std::min(cx, cy) - ui.h(0.12f);
                    tft.fillCircle(cx, cy, r, 0x0000);

                    char valBuf[32];
                    if (state.monitors[mIdx].decimals == 2) {
                        snprintf(valBuf, sizeof(valBuf), "%+.2f", val);
                    } else {
                        snprintf(valBuf, sizeof(valBuf), "%+.1f", val);
                    }

                    ui.textCenter(state.monitors[mIdx].title, 0xFFFF, 0.08f, 0.23f);
                    ui.textCenter(valBuf, color, 0.18f, 0.50f);
                    ui.circularBar(val, currentLimit, color, 0x7BEF, 0.1f);
                } else {
                    // Differential update at 25Hz - instant, zero flicker, zero sweeping animation!
                    if (std::abs(val - lastVal) >= 0.01f || std::abs(angle - lastAngle) >= 0.5f) {
                        char valBuf[32];
                        if (state.monitors[mIdx].decimals == 2) {
                            snprintf(valBuf, sizeof(valBuf), "%+.2f", val);
                        } else {
                            snprintf(valBuf, sizeof(valBuf), "%+.1f", val);
                        }

                        // Clear center text area only
                        int cx = tft.width() / 2;
                        int cy = tft.height() / 2;
                        int r = std::min(cx, cy) - ui.h(0.12f);
                        tft.fillCircle(cx, cy, r, 0x0000);

                        ui.textCenter(state.monitors[mIdx].title, 0xFFFF, 0.08f, 0.23f);
                        ui.textCenter(valBuf, color, 0.18f, 0.50f);

                        ui.circularBarDiff(lastAngle, angle, color, 0x7BEF, 0.1f);
                        lastAngle = angle;
                        lastVal = val;
                    }
                }
            } else {
                tft.fillScreen(0x0000);
                ui.textCenter(state.monitors[mIdx].title, 0xFFFF, 0.08f, 0.23f);
                ui.textCenter("---", 0xFFFF, 0.18f, 0.50f);
                ui.circularBar(0, currentLimit, 0x0000, 0x7BEF, 0.1f);
            }
        }
    }
};
