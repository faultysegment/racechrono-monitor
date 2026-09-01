#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"

extern Arduino_DataBus *g_amoled_bus;
extern Arduino_GFX *g_amoled_gfx;

class AmoledDisplayPolicy {
public:
    void init() {
        if (!g_amoled_bus) {
            g_amoled_bus = new Arduino_ESP32QSPI(
                LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
        }
        if (!g_amoled_gfx) {
#if defined(DO0143FAT01)
            g_amoled_gfx = new Arduino_SH8601(g_amoled_bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT);
#elif defined(H0175Y003AM) || defined(DO0143FMST10)
            g_amoled_gfx = new Arduino_CO5300(g_amoled_bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT, 6, 0, 0, 0);
#endif
        }

        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, HIGH);

        g_amoled_gfx->begin();
        g_amoled_gfx->fillScreen(0x0000);

        for (int i = 0; i <= 200; i++) {
            g_amoled_gfx->Display_Brightness(i);
            delay(2);
        }
    }

    void setRotation(uint8_t r) {}
    void setHudMode(bool hud) {
        static bool currentHud = false;
        if (currentHud == hud) return;
        currentHud = hud;

        if (g_amoled_bus) {
            g_amoled_bus->beginWrite();
            g_amoled_bus->writeC8D8(0x36, hud ? 0x02 : 0x00);
            g_amoled_bus->endWrite();
        }
    }

    void fillScreen(uint16_t color) { if (g_amoled_gfx) g_amoled_gfx->fillScreen(color); }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { if (g_amoled_gfx) g_amoled_gfx->fillRect(x, y, w, h, color); }
    void setCursor(int16_t x, int16_t y) { if (g_amoled_gfx) g_amoled_gfx->setCursor(x, y); }
    void setTextColor(uint16_t c) { if (g_amoled_gfx) g_amoled_gfx->setTextColor(c); }
    void setTextColor(uint16_t c, uint16_t bg) { if (g_amoled_gfx) g_amoled_gfx->setTextColor(c, bg); }
    void setTextSize(uint8_t s) { if (g_amoled_gfx) g_amoled_gfx->setTextSize(s); }
    void setTextWrap(bool w) { if (g_amoled_gfx) g_amoled_gfx->setTextWrap(w); }
    size_t print(const char* str) { return g_amoled_gfx ? g_amoled_gfx->print(str) : 0; }
    size_t print(int n) { return g_amoled_gfx ? g_amoled_gfx->print(n) : 0; }
    size_t println(const char* str) { return g_amoled_gfx ? g_amoled_gfx->println(str) : 0; }

    int16_t textWidth(const char* str) {
        if (!g_amoled_gfx) return 0;
        int16_t x1, y1;
        uint16_t w, h;
        g_amoled_gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
        return w;
    }

    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) { if (g_amoled_gfx) g_amoled_gfx->fillCircle(x, y, r, color); }
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { if (g_amoled_gfx) g_amoled_gfx->drawFastHLine(x, y, w, color); }
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { if (g_amoled_gfx) g_amoled_gfx->drawFastVLine(x, y, h, color); }

    int16_t width() { return g_amoled_gfx ? g_amoled_gfx->width() : LCD_WIDTH; }
    int16_t height() { return g_amoled_gfx ? g_amoled_gfx->height() : LCD_HEIGHT; }

    void setBacklight(bool on) {
        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, on ? HIGH : LOW);
        if (g_amoled_gfx) g_amoled_gfx->Display_Brightness(on ? 200 : 0);
    }

    void drawBattery(int percent, bool force = false) {}
    void flush() {}
};
