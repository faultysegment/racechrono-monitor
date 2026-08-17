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
        int targetPixels = tft.height() * percent;
        return std::max(1, (int)std::round((float)targetPixels / getBaseTextHeight()));
    }

    void textCenter(const char* str, uint32_t color, float sizePercent, float offsetYPercent = 0.5f, uint32_t bg = 0x0000) {
        tft.setTextColor(color, bg);
        tft.setTextSize(textSize(sizePercent));
        
        int strW = tft.textWidth(str);
        int strH = getBaseTextHeight() * textSize(sizePercent);
        
        int cursorX = (tft.width() - strW) / 2;
        int cursorY = h(offsetYPercent) - (strH / 2);
        
        tft.setCursor(cursorX, cursorY);
        tft.print(str);
    }

    void circularBarDiff(float oldAngle, float newAngle, uint32_t filledColor, uint32_t bgColor, float thicknessPercent = 0.1f) {
        int cx = tft.width() / 2;
        int cy = tft.height() / 2;
        int radiusOut = std::min(cx, cy) - 5;
        int radiusIn = radiusOut - h(thicknessPercent);

        if (newAngle > oldAngle) {
            tft.fillArc(cx, cy, radiusOut, radiusIn, oldAngle, newAngle, filledColor);
        } else if (newAngle < oldAngle) {
            tft.fillArc(cx, cy, radiusOut, radiusIn, newAngle, oldAngle, bgColor);
        }
    }

    void circularBar(float value, float limit, uint32_t filledColor, uint32_t bgColor, float thicknessPercent = 0.1f) {
        int cx = tft.width() / 2;
        int cy = tft.height() / 2;
        int radiusOut = std::min(cx, cy) - 5;
        int radiusIn = radiusOut - h(thicknessPercent);

        float absVal = std::abs(value);
        if (absVal > limit) absVal = limit;
        
        float angle = (absVal / limit) * 360.0f;

        // Draw background arc
        tft.fillArc(cx, cy, radiusOut, radiusIn, angle, 360.0f, bgColor);
        
        // Draw filled arc
        if (angle > 0) {
            tft.fillArc(cx, cy, radiusOut, radiusIn, 0.0f, angle, filledColor);
        }
    }

    void drawPlusMinus(uint32_t color) {
        int cx = tft.width() / 2;
        int cy = tft.height() / 2;
        
        tft.setTextColor(color, 0x0000);
        tft.setTextSize(textSize(0.15f));
        
        // Draw Minus on the left
        tft.setCursor(cx - w(0.35f), cy - h(0.07f));
        tft.print("-");
        
        // Draw Plus on the right
        tft.setCursor(cx + w(0.28f), cy - h(0.07f));
        tft.print("+");
    }

    void drawCheckmark(uint32_t color) {
        int cx = tft.width() / 2;
        int cy = tft.height() - h(0.2f);
        
        tft.setTextColor(color, 0x0000);
        tft.setTextSize(textSize(0.1f));
        
        int strW = tft.textWidth("OK");
        tft.setCursor(cx - (strW/2), cy);
        tft.print("OK");
    }
};
