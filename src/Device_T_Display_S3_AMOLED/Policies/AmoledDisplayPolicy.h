#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"

extern Arduino_DataBus *bus;
extern Arduino_GFX *gfx;

class AmoledDisplayPolicy {
public:
    void init() {
        if (!bus) {
            bus = new Arduino_ESP32QSPI(
                LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
                LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);
        }
        if (!gfx) {
#if defined DO0143FAT01
            gfx = new Arduino_SH8601(bus, LCD_RST /* RST */,
                                    0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT);
#elif (defined DO0143FMST10) || (defined H0175Y003AM)
            gfx = new Arduino_CO5300(bus, LCD_RST /* RST */,
                                    0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT,
                                    6 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);
#endif
        }

#if defined(LCD_EN)
        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, HIGH);
        delay(50); // wait for power to stabilize
#endif

        gfx->begin();
    }
    
    bool isHudMode = false;
    void setRotation(uint8_t r) { gfx->setRotation(r); }
    void setHudMode(bool hud) {
        if (isHudMode != hud) {
            isHudMode = hud;
            gfx->setRotation(hud ? 1 : 0);
        }
    }
    void fillScreen(uint32_t color) { gfx->fillScreen(color); }
    void setCursor(int16_t x, int16_t y) { gfx->setCursor(x, y); }
    void setTextWrap(bool wrap) { gfx->setTextWrap(wrap); }
    void setTextSize(uint8_t size) { gfx->setTextSize(size); }
    void setTextColor(uint32_t c, uint32_t bg) { gfx->setTextColor(c, bg); }
    void print(const char* str) { gfx->print(str); }
    void print(int n) { gfx->print(n); }
    void println(const char* str) { gfx->println(str); }
    
    int16_t textWidth(const char* str) { 
        int16_t x1, y1;
        uint16_t w, h;
        gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
        return w;
    }
    
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) { gfx->fillRect(x, y, w, h, color); }
    void fillArc(int16_t x, int16_t y, int16_t r1, int16_t r2, float start, float end, uint16_t color) {
        gfx->fillArc(x, y, r1, r2, start, end, color);
    }
    void fillCircle(int16_t x, int16_t y, int16_t r, uint32_t color) { gfx->fillCircle(x, y, r, color); }
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint32_t color) { gfx->drawFastHLine(x, y, w, color); }
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint32_t color) { gfx->drawFastVLine(x, y, h, color); }
    
    int16_t width() { return gfx->width(); }
    int16_t height() { return gfx->height(); }
    void flush() { gfx->flush(); }
    
    void setBacklight(bool on) {
#if defined(LCD_EN)
        digitalWrite(LCD_EN, on ? HIGH : LOW);
#endif
        if (gfx) {
            gfx->Display_Brightness(on ? 255 : 0);
        }
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
