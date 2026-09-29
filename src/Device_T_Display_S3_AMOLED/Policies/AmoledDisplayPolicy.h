#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include "pin_config.h"
#include "../../Device_All/DisplayRounder.h"

class AmoledDisplayPolicy {
    Arduino_DataBus *bus = nullptr;
    Arduino_GFX *gfx = nullptr;
    bool currentHud = false;

    static const uint32_t BUF_SIZE = LCD_WIDTH * 40;
    lv_color_t* disp_buf = nullptr;
    lv_disp_draw_buf_t draw_buf;
    lv_disp_drv_t disp_drv;
    lv_disp_t* disp = nullptr;

    static void flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
        auto* self = static_cast<AmoledDisplayPolicy*>(drv->user_data);
        if (!self || !self->gfx) return;

        uint32_t w = (area->x2 - area->x1 + 1);
        uint32_t h = (area->y2 - area->y1 + 1);

        self->gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t*)&color_p->full, w, h);

        lv_disp_flush_ready(drv);
    }

public:
    AmoledDisplayPolicy() = default;
    ~AmoledDisplayPolicy() {
        if (disp_buf) {
            free(disp_buf);
            disp_buf = nullptr;
        }
        if (gfx) { delete gfx; gfx = nullptr; }
        if (bus) { delete bus; bus = nullptr; }
    }

    void init() {
        if (!bus) {
            bus = new Arduino_ESP32QSPI(
                LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
        }
        if (!gfx) {
#if defined(DO0143FAT01)
            gfx = new Arduino_SH8601(bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT);
#elif defined(H0175Y003AM) || defined(DO0143FMST10)
            gfx = new Arduino_CO5300(bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT, 6, 0, 0, 0);
#endif
        }

        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, HIGH);

        if (gfx) {
            gfx->begin();
            gfx->fillScreen(0x0000);

            for (int i = 0; i <= 200; i++) {
                gfx->Display_Brightness(i);
                delay(2);
            }
        }

        if (!disp_buf) {
            disp_buf = (lv_color_t*)malloc(BUF_SIZE * sizeof(lv_color_t));
            lv_disp_draw_buf_init(&draw_buf, disp_buf, NULL, BUF_SIZE);

            lv_disp_drv_init(&disp_drv);
            disp_drv.hor_res = LCD_WIDTH;
            disp_drv.ver_res = LCD_HEIGHT;
            disp_drv.flush_cb = flush_cb;
            disp_drv.rounder_cb = DisplayRounder::roundToEven;
            disp_drv.draw_buf = &draw_buf;
            disp_drv.user_data = this;

            disp = lv_disp_drv_register(&disp_drv);
        }
    }

    void setRotation(uint8_t r) {}

    void setHudMode(bool hud) {
        if (currentHud == hud) return;
        currentHud = hud;

        if (bus) {
            bus->beginWrite();
            bus->writeC8D8(0x36, hud ? 0x02 : 0x00);
            bus->endWrite();
        }
    }

    void fillScreen(uint16_t color) { if (gfx) gfx->fillScreen(color); }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { if (gfx) gfx->fillRect(x, y, w, h, color); }
    void setCursor(int16_t x, int16_t y) { if (gfx) gfx->setCursor(x, y); }
    void setTextColor(uint16_t c) { if (gfx) gfx->setTextColor(c); }
    void setTextColor(uint16_t c, uint16_t bg) { if (gfx) gfx->setTextColor(c, bg); }
    void setTextSize(uint8_t s) { if (gfx) gfx->setTextSize(s); }
    void setTextWrap(bool wrap) { if (gfx) gfx->setTextWrap(wrap); }
    size_t print(const char* str) { return gfx ? gfx->print(str) : 0; }
    size_t print(int n) { return gfx ? gfx->print(n) : 0; }
    size_t println(const char* str) { return gfx ? gfx->println(str) : 0; }

    int16_t textWidth(const char* str) {
        if (!gfx) return 0;
        int16_t x1, y1;
        uint16_t w, h;
        gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
        return w;
    }

    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) { if (gfx) gfx->fillCircle(x, y, r, color); }
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { if (gfx) gfx->drawFastHLine(x, y, w, color); }
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { if (gfx) gfx->drawFastVLine(x, y, h, color); }

    int16_t width() { return gfx ? gfx->width() : LCD_WIDTH; }
    int16_t height() { return gfx ? gfx->height() : LCD_HEIGHT; }

    void setBacklight(bool on) {
        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, on ? HIGH : LOW);
        if (gfx) gfx->Display_Brightness(on ? 200 : 0);
    }

    void drawBattery(int percent, bool force = false) {}
    void flush() {}
};
