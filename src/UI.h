#pragma once
#include <cmath>
#include <cstdio>

template <typename DisplayPolicy>
class UI {
    DisplayPolicy& tft;
    int cursorX;
    int cursorY;

public:
    UI(DisplayPolicy& display) : tft(display), cursorX(0), cursorY(0) {}

    void begin() {
        cursorX = 0;
        cursorY = 0;
    }

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

    void setCursorY(float percentY) {
        cursorY = h(percentY);
    }
    
    int getCursorY() const {
        return cursorY;
    }

    void space(float percentY) {
        cursorY += h(percentY);
    }

    void textCenter(const char* str, uint32_t color, float sizePercent, float offsetXPercent = 0.0f, uint32_t bg = 0x0000) {
        tft.setTextColor(color, bg);
        tft.setTextSize(textSize(sizePercent));
        
        int strW = tft.textWidth(str);
        int strCursorX = (tft.width() - strW) / 2 + w(offsetXPercent);
        
        tft.setCursor(strCursorX, cursorY);
        tft.print(str);
    }

    void textLeft(const char* str, uint32_t color, float sizePercent, float offsetXPercent = 0.0f) {
        tft.setTextColor(color, 0x0000);
        tft.setTextSize(textSize(sizePercent));
        tft.setCursor(cursorX + w(offsetXPercent), cursorY);
        tft.print(str);
    }

    void bar(float value, float limit, uint32_t filledColor, uint32_t bgColor, float heightPercent = 0.23f) {
        int maxW = tft.width();
        int heightPx = h(heightPercent);
        
        float absVal = std::abs(value);
        if (absVal > limit) absVal = limit;
        int fillW = (int)((absVal / limit) * maxW);
        
        tft.fillRect(0, cursorY, fillW, heightPx, filledColor);
        tft.fillRect(fillW, cursorY, maxW - fillW, heightPx, bgColor);
        
        cursorY += heightPx;
    }

    void emptyBar(uint32_t bgColor, float heightPercent = 0.23f) {
        int heightPx = h(heightPercent);
        int maxW = tft.width();
        tft.fillRect(0, cursorY, maxW, heightPx, bgColor);
        cursorY += heightPx;
    }
};
