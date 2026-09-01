#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"

class AmoledDisplayPolicy {
private:
    static Arduino_DataBus*& getBus() { static Arduino_DataBus* bus = nullptr; return bus; }
    static Arduino_GFX*& getGfx() { static Arduino_GFX* gfx = nullptr; return gfx; }
    static bool& getHudMode() { static bool hud = false; return hud; }

public:
    void init() {
        Arduino_DataBus*& bus = getBus();
        Arduino_GFX*& gfx = getGfx();

        if (!bus) {
            bus = new Arduino_ESP32QSPI(
                LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
        }
        if (!gfx) {
#if defined DO0143FAT01
            gfx = new Arduino_SH8601(bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT);
#elif (defined DO0143FMST10) || (defined H0175Y003AM)
            gfx = new Arduino_CO5300(bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT, 6, 0, 0, 0);
#endif
        }

#if defined(LCD_EN)
        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, HIGH);
        delay(50);
#endif

        gfx->begin();
    }

    void setRotation(uint8_t r) {
        Arduino_GFX* gfx = getGfx();
        if (gfx) gfx->setRotation(r);
    }

    void setHudMode(bool hud) {
        bool& currentHud = getHudMode();
        if (currentHud != hud) {
            currentHud = hud;
            Arduino_DataBus* bus = getBus();
            if (bus) {
                bus->beginWrite();
                bus->writeC8D8(0x36, hud ? 0x02 : 0x00);
                bus->endWrite();
            }
        }
    }

    void fillScreen(uint32_t color) { if (getGfx()) getGfx()->fillScreen(color); }
    void setCursor(int16_t x, int16_t y) { if (getGfx()) getGfx()->setCursor(x, y); }
    void setTextWrap(bool wrap) { if (getGfx()) getGfx()->setTextWrap(wrap); }
    void setTextSize(uint8_t size) { if (getGfx()) getGfx()->setTextSize(size); }
    void setTextColor(uint32_t c) { if (getGfx()) getGfx()->setTextColor(c); }
    void setTextColor(uint32_t c, uint32_t bg) { if (getGfx()) getGfx()->setTextColor(c, bg); }
    void print(const char* str) { if (getGfx()) getGfx()->print(str); }
    void print(int n) { if (getGfx()) getGfx()->print(n); }
    void println(const char* str) { if (getGfx()) getGfx()->println(str); }

    int16_t textWidth(const char* str) {
        Arduino_GFX* gfx = getGfx();
        if (!gfx) return 0;
        int16_t x1, y1;
        uint16_t w, h;
        gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
        return w;
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) { if (getGfx()) getGfx()->fillRect(x, y, w, h, color); }
    void fillCircle(int16_t x, int16_t y, int16_t r, uint32_t color) { if (getGfx()) getGfx()->fillCircle(x, y, r, color); }
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint32_t color) { if (getGfx()) getGfx()->drawFastHLine(x, y, w, color); }
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint32_t color) { if (getGfx()) getGfx()->drawFastVLine(x, y, h, color); }

    int16_t width() {
        Arduino_GFX* gfx = getGfx();
        return gfx ? gfx->width() : LCD_WIDTH;
    }

    int16_t height() {
        Arduino_GFX* gfx = getGfx();
        return gfx ? gfx->height() : LCD_HEIGHT;
    }

    void flush() {
        // Direct drawing to gfx hardware QSPI bus, no full-framebuffer copy needed
    }

    void setBacklight(bool on) {
#if defined(LCD_EN)
        digitalWrite(LCD_EN, on ? HIGH : LOW);
#endif
        Arduino_GFX* gfx = getGfx();
        if (gfx) gfx->Display_Brightness(on ? 255 : 0);
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
