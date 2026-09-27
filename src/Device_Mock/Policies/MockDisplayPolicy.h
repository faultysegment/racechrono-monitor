#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <cstring>
#include <lvgl.h>

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

struct MockCircle {
    int16_t x, y, r;
    uint32_t color;
};

class MockDisplayPolicy {
    lv_disp_drv_t disp_drv;
    lv_disp_draw_buf_t draw_buf;
    lv_color_t buf[320 * 20];
    bool initialized = false;

    static void flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
        auto* self = static_cast<MockDisplayPolicy*>(drv->user_data);
        if (self) {
            self->lastFlushedArea = *area;
        }
        lv_disp_flush_ready(drv);
    }

public:
    std::string lastPrint = "";
    uint32_t lastFillScreenColor = 0;
    std::vector<MockRect> lastRects;
    std::vector<MockCircle> lastCircles;
    uint32_t lastTextColor = 0;
    std::vector<uint32_t> allTextColors;
    bool isHud = false;
    int currentTextSize = 1;
    lv_area_t lastFlushedArea = {0, 0, 0, 0};

    void reset() {
        lastPrint = "";
        lastFillScreenColor = 0;
        lastRects.clear();
        lastCircles.clear();
        lastTextColor = 0;
        allTextColors.clear();
        isHud = false;
        currentTextSize = 1;
    }

    void init() {
        if (!initialized) {
            if (!lv_is_initialized()) {
                lv_init();
            }
            lv_disp_draw_buf_init(&draw_buf, buf, NULL, 320 * 20);
            lv_disp_drv_init(&disp_drv);
            disp_drv.draw_buf = &draw_buf;
            disp_drv.flush_cb = flush_cb;
            disp_drv.hor_res = 320;
            disp_drv.ver_res = 170;
            disp_drv.user_data = this;
            lv_disp_drv_register(&disp_drv);
            initialized = true;
        }
    }

    void setRotation(uint8_t r) {}
    void setHudMode(bool hud) { isHud = hud; }
    void fillScreen(uint32_t color) { lastFillScreenColor = color; }
    void setCursor(int16_t x, int16_t y) {}
    void setTextWrap(bool wrap) {}
    void setTextSize(uint8_t size) { currentTextSize = size; }
    int16_t textWidth(const char* str) { return strlen(str) * 6 * currentTextSize; }
    void setTextColor(uint32_t c) { lastTextColor = c; allTextColors.push_back(c); }
    void setTextColor(uint32_t c, uint32_t bg) { lastTextColor = c; allTextColors.push_back(c); }
    void print(const char* str) { lastPrint += str; }
    void print(int n) { lastPrint += std::to_string(n); }
    void println(const char* str) { lastPrint += str; lastPrint += "\n"; }
    void fillCircle(int16_t x, int16_t y, int16_t r, uint32_t color) {
        lastCircles.push_back({x, y, r, color});
    }

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
