#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"

class AmoledDisplayPolicy {
private:
    static Arduino_DataBus*& getBus() { static Arduino_DataBus* bus = nullptr; return bus; }
    static Arduino_GFX*& getGfx() { static Arduino_GFX* gfx = nullptr; return gfx; }
    static Arduino_Canvas*& getCanvas() { static Arduino_Canvas* canvas = nullptr; return canvas; }
    static bool& getHudMode() { static bool hud = false; return hud; }
    static bool& getDirty() { static bool dirty = false; return dirty; }

public:
    void init() {
        Arduino_DataBus*& bus = getBus();
        Arduino_GFX*& gfx = getGfx();
        Arduino_Canvas*& canvas = getCanvas();

        if (!bus) {
            bus = new Arduino_ESP32QSPI(
                LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
        }
        if (!gfx) {
#if defined H0175Y003AM
            gfx = new Arduino_CO5300(bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT, 0, 0, 0, 0);
#elif defined DO0143FAT01
            gfx = new Arduino_SH8601(bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT);
#elif defined DO0143FMST10
            gfx = new Arduino_CO5300(bus, LCD_RST, 0, false, LCD_WIDTH, LCD_HEIGHT, 0, 0, 0, 0);
#endif
        }

#if defined(LCD_EN)
        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, HIGH);
        delay(50);
#endif

        gfx->begin(80000000);
        gfx->fillScreen(0x0000);
        gfx->Display_Brightness(0x00);

        if (!canvas) {
            canvas = new Arduino_Canvas(LCD_WIDTH, LCD_HEIGHT, gfx);
            canvas->begin(GFX_SKIP_OUTPUT_BEGIN);
            canvas->fillScreen(0x0000);
            getDirty() = true;
            canvas->flush();
            getDirty() = false;
        }
    }

    void setRotation(uint8_t r) {
        Arduino_Canvas* canvas = getCanvas();
        if (canvas) canvas->setRotation(0);
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

    void fillScreen(uint32_t color) { if (getCanvas()) { getCanvas()->fillScreen(color); getDirty() = true; } }
    void setCursor(int16_t x, int16_t y) { if (getCanvas()) getCanvas()->setCursor(x, y); }
    void setTextWrap(bool wrap) { if (getCanvas()) getCanvas()->setTextWrap(wrap); }
    void setTextSize(uint8_t size) { if (getCanvas()) getCanvas()->setTextSize(size); }
    void setTextColor(uint32_t c) { if (getCanvas()) getCanvas()->setTextColor(c); }
    void setTextColor(uint32_t c, uint32_t bg) { if (getCanvas()) getCanvas()->setTextColor(c, bg); }
    void print(const char* str) { if (getCanvas()) { getCanvas()->print(str); getDirty() = true; } }
    void print(int n) { if (getCanvas()) { getCanvas()->print(n); getDirty() = true; } }
    void println(const char* str) { if (getCanvas()) { getCanvas()->println(str); getDirty() = true; } }

    int16_t textWidth(const char* str) {
        Arduino_Canvas* canvas = getCanvas();
        if (!canvas) return 0;
        int16_t x1, y1;
        uint16_t w, h;
        canvas->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
        return w;
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) { if (getCanvas()) { getCanvas()->fillRect(x, y, w, h, color); getDirty() = true; } }
    void fillCircle(int16_t x, int16_t y, int16_t r, uint32_t color) { if (getCanvas()) { getCanvas()->fillCircle(x, y, r, color); getDirty() = true; } }
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint32_t color) { if (getCanvas()) { getCanvas()->drawFastHLine(x, y, w, color); getDirty() = true; } }
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint32_t color) { if (getCanvas()) { getCanvas()->drawFastVLine(x, y, h, color); getDirty() = true; } }

    int16_t width() {
        Arduino_Canvas* canvas = getCanvas();
        return canvas ? canvas->width() : LCD_WIDTH;
    }

    int16_t height() {
        Arduino_Canvas* canvas = getCanvas();
        return canvas ? canvas->height() : LCD_HEIGHT;
    }

    void flush() {
        if (!getDirty()) return;
        getDirty() = false;
        Arduino_Canvas* canvas = getCanvas();
        if (canvas) canvas->flush();
    }

    void setBacklight(bool on) {
#if defined(LCD_EN)
        digitalWrite(LCD_EN, on ? HIGH : LOW);
#endif
        Arduino_GFX* gfx = getGfx();
        if (gfx) gfx->Display_Brightness(on ? 0x00 : 0xFF);
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
