#pragma once
#include <cmath>
#include <cstdio>
#include <algorithm>

template <typename DisplayPolicy>
class CircularUI {
    DisplayPolicy& tft;

public:
    CircularUI(DisplayPolicy& display) : tft(display) {}

    void begin() {}

    int h(float percent) {
        return (int)(tft.height() * percent);
    }

    int w(float percent) {
        return (int)(tft.width() * percent);
    }

    int getBaseTextHeight() {
        return tft.height() > 320 ? 16 : 8;
    }

    int textSize(float percent) {
        int targetPixels = (int)(tft.height() * percent);
        return std::max(1, targetPixels / 8);
    }

    void textCenter(const char* str, uint32_t color, float sizePercent, float offsetYPercent = 0.5f) {
        int sz = textSize(sizePercent);
        tft.setTextSize(sz);
        tft.setTextColor(color);
        
        int strW = strlen(str) * 6 * sz;
        int strH = 8 * sz;
        
        int cursorX = (tft.width() - strW) / 2;
        int cursorY = h(offsetYPercent) - (strH / 2);
        
        tft.setCursor(cursorX, cursorY);
        tft.print(str);
    }

    void textCenter(const char* str, uint32_t color, float sizePercent, float offsetYPercent, uint32_t bg) {
        int sz = textSize(sizePercent);
        int strW = strlen(str) * 6 * sz;
        int strH = 8 * sz;
        
        int cursorX = (tft.width() - strW) / 2;
        int cursorY = h(offsetYPercent) - (strH / 2);
        
        tft.fillRect(cursorX, cursorY, strW, strH, bg);
        tft.setTextSize(sz);
        tft.setTextColor(color);
        tft.setCursor(cursorX, cursorY);
        tft.print(str);
    }

    void circularRadialBar(float value, float limit, uint32_t filledColor, uint32_t bgColor) {
        int cx = tft.width() / 2;
        int cy = tft.height() / 2;
        int radiusOut = std::min(cx, cy) - 5;
        
        float absVal = std::abs(value);
        if (absVal > limit) absVal = limit;
        float pct = (limit > 0.0001f) ? (absVal / limit) : 0.0f;
        if (pct > 1.0f) pct = 1.0f;

        const int maxThickness = std::max(12, (int)std::round(radiusOut * 0.15f));
        const int minThickness = 4;

        if (pct <= 0.001f) {
            tft.fillCircle(cx, cy, radiusOut, 0x0000);
        } else {
            if (pct < 0.10f) {
                pct = 0.10f;
            }
            int currentThickness = minThickness + (int)std::round((maxThickness - minThickness) * pct);
            int rIn = radiusOut - currentThickness;
            // Fill from outer edge (radiusOut) inward to rIn
            tft.fillCircle(cx, cy, radiusOut, filledColor);
            if (rIn > 0) {
                tft.fillCircle(cx, cy, rIn, 0x0000);
            }
        }
    }
};
