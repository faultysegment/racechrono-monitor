#pragma once

#include <TFT_eSPI.h>
#include <lvgl.h>

class RealDisplayPolicy {
    TFT_eSPI tft;
    bool isHudMode = false;

    static const uint32_t BUF_SIZE = 320 * 20;
    lv_color_t* disp_buf = nullptr;
    lv_disp_draw_buf_t draw_buf;
    lv_disp_drv_t disp_drv;
    lv_disp_t* disp = nullptr;

    static void flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
        auto* self = static_cast<RealDisplayPolicy*>(drv->user_data);
        if (!self) return;

        uint32_t w = (area->x2 - area->x1 + 1);
        uint32_t h = (area->y2 - area->y1 + 1);

        self->tft.startWrite();
        self->tft.setAddrWindow(area->x1, area->y1, w, h);
        self->tft.pushColors((uint16_t*)&color_p->full, w * h, true);
        self->tft.endWrite();

        lv_disp_flush_ready(drv);
    }

public:
    RealDisplayPolicy() = default;
    ~RealDisplayPolicy() {
        if (disp_buf) {
            free(disp_buf);
            disp_buf = nullptr;
        }
    }

    void init() {
        tft.init();
        tft.setRotation(3);

        if (!disp_buf) {
            disp_buf = (lv_color_t*)malloc(BUF_SIZE * sizeof(lv_color_t));
            lv_disp_draw_buf_init(&draw_buf, disp_buf, NULL, BUF_SIZE);

            lv_disp_drv_init(&disp_drv);
            disp_drv.hor_res = 320;
            disp_drv.ver_res = 170;
            disp_drv.flush_cb = flush_cb;
            disp_drv.draw_buf = &draw_buf;
            disp_drv.user_data = this;

            disp = lv_disp_drv_register(&disp_drv);
        }
    }

    void setRotation(uint8_t r) { tft.setRotation(r); }

    void setHudMode(bool hud) {
        if (isHudMode != hud) {
            isHudMode = hud;
            tft.writecommand(TFT_MADCTL);
            if (hud) {
                tft.writedata(TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
            } else {
                tft.writedata(TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
            }
        }
    }

    void fillScreen(uint32_t color) { tft.fillScreen(color); }
    void setCursor(int16_t x, int16_t y) { tft.setCursor(x, y); }
    void setTextWrap(bool wrap) { tft.setTextWrap(wrap); }
    void setTextSize(uint8_t size) { tft.setTextSize(size); }
    void setTextColor(uint32_t c) { tft.setTextColor(c); }
    void setTextColor(uint32_t c, uint32_t bg) { tft.setTextColor(c, bg); }
    void print(const char* str) { tft.print(str); }
    void print(int n) { tft.print(n); }
    void println(const char* str) { tft.println(str); }
    int16_t textWidth(const char* str) { return tft.textWidth(str); }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) { tft.fillRect(x, y, w, h, color); }
    int16_t width() { return 320; }
    int16_t height() { return 170; }
    void flush() {}

    void setBacklight(bool on) {
#ifdef TFT_BL
        ::pinMode(TFT_BL, OUTPUT);
        ::digitalWrite(TFT_BL, on ? HIGH : LOW);
#endif
    }

    void drawBattery(int percent, bool force = false) {}
};
