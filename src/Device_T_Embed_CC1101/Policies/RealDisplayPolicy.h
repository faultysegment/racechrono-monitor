#pragma once

#include <TFT_eSPI.h>

class RealDisplayPolicy {
    TFT_eSPI tft;
public:
    void init() { tft.init(); }
    void setRotation(uint8_t r) { tft.setRotation(r); }
    void fillScreen(uint32_t color) { tft.fillScreen(color); }
    void setCursor(int16_t x, int16_t y) { tft.setCursor(x, y); }
    void setTextWrap(bool wrap) { tft.setTextWrap(wrap); }
    void setTextSize(uint8_t size) { tft.setTextSize(size); }
    void setTextColor(uint32_t c, uint32_t bg) { tft.setTextColor(c, bg); }
    void print(const char* str) { tft.print(str); }
    void print(int n) { tft.print(n); }
    void println(const char* str) { tft.println(str); }
    int16_t textWidth(const char* str) { return tft.textWidth(str); }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) { tft.fillRect(x, y, w, h, color); }
    int16_t width() { return tft.width(); }
    int16_t height() { return tft.height(); }
    void flush() {}
    
    void setBacklight(bool on) {
#ifdef TFT_BL
        ::pinMode(TFT_BL, OUTPUT);
        ::digitalWrite(TFT_BL, on ? HIGH : LOW);
#endif
    }

    void drawBattery(int percent, bool force = false) {
        static int lastBat = -2;
        if (force || lastBat != percent) {
            lastBat = percent;
            int screenW = width();
            setTextSize(2);
            setTextColor(0xFFFF, 0x0000); 
            setCursor(screenW - 55, 10); 
            char buf[16];
            if (percent >= 0 && percent <= 100) {
                snprintf(buf, sizeof(buf), "%3d%%", percent);
            } else {
                snprintf(buf, sizeof(buf), "---%%");
            }
            print(buf);
        }
    }
};
