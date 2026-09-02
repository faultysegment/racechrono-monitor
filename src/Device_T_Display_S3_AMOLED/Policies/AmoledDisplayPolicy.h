#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"

class AmoledDisplayPolicy {
    Arduino_DataBus *bus = nullptr;
    Arduino_GFX *gfx = nullptr;
    bool currentHud = false;

public:
    AmoledDisplayPolicy() = default;
    ~AmoledDisplayPolicy() {
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
    void setTextWrap(bool w) { if (gfx) gfx->setTextWrap(w); }
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
