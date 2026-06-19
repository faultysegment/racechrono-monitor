#pragma once

#ifndef PIO_UNIT_TESTING
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
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) { tft.fillRect(x, y, w, h, color); }
    int16_t width() { return tft.width(); }
    int16_t height() { return tft.height(); }
};
#endif
