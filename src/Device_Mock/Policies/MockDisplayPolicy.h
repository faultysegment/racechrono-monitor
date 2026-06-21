#pragma once
#include <cstdint>
#include <string>
#include <vector>

#define TFT_BLACK       0x0000
#define TFT_BLUE        0x001F
#define TFT_RED         0xF800
#define TFT_GREEN       0x07E0
#define TFT_WHITE       0xFFFF
#define TFT_LIGHTGREY   0xD69A
#define TFT_DARKGREY    0x7BEF

struct MockRect {
    int16_t x, y, w, h;
    uint32_t color;
};

class MockDisplayPolicy {
public:
    static std::string lastPrint;
    static uint32_t lastFillScreenColor;
    static std::vector<MockRect> lastRects;

    static void reset() {
        lastPrint = "";
        lastFillScreenColor = 0;
        lastRects.clear();
    }

    void init() {}
    void setRotation(uint8_t r) {}
    void fillScreen(uint32_t color) { lastFillScreenColor = color; }
    void setCursor(int16_t x, int16_t y) {}
    void setTextWrap(bool wrap) {}
    void setTextSize(uint8_t size) {}
    void setTextColor(uint32_t c, uint32_t bg) {}
    void print(const char* str) { lastPrint += str; }
    void print(int n) { lastPrint += std::to_string(n); }
    void println(const char* str) { lastPrint += str; lastPrint += "\n"; }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) {
        lastRects.push_back({x, y, w, h, color});
    }
    int16_t width() { return 320; }
    int16_t height() { return 170; }
    void flush() {}
    void setBacklight(bool on) {}
    void drawBattery(int percent, bool force = false) {
        lastPrint += "Bat:" + std::to_string(percent) + "%";
    }
};

#ifdef PIO_UNIT_TESTING
std::string MockDisplayPolicy::lastPrint = "";
uint32_t MockDisplayPolicy::lastFillScreenColor = 0;
std::vector<MockRect> MockDisplayPolicy::lastRects;
#endif
