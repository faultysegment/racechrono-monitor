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
    void setRotation(uint8_t r) {
        if (gfx) gfx->setRotation(r);
    }
    
    void setHudMode(bool hud) {
        if (isHudMode != hud) {
            isHudMode = hud;
            if (bus) {
                bus->beginWrite();
                bus->writeC8D8(0x36, hud ? 0x02 : 0x00); // MADCTL 0x36: 0x02 = X_AXIS_FLIP (Horizontal Mirror)
                bus->endWrite();
            }
        }
    }
    
    void fillScreen(uint32_t color) { if (gfx) gfx->fillScreen(color); }
    void setCursor(int16_t x, int16_t y) { if (gfx) gfx->setCursor(x, y); }
    void setTextWrap(bool wrap) { if (gfx) gfx->setTextWrap(wrap); }
    void setTextSize(uint8_t size) { if (gfx) gfx->setTextSize(size); }
    void setTextColor(uint32_t c, uint32_t bg) { if (gfx) gfx->setTextColor(c, bg); }
    void print(const char* str) { if (gfx) gfx->print(str); }
    void print(int n) { if (gfx) gfx->print(n); }
    void println(const char* str) { if (gfx) gfx->println(str); }
    
    int16_t textWidth(const char* str) { 
        if (!gfx) return 0;
        int16_t x1, y1;
        uint16_t w, h;
        gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
        return w;
    }
    
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) { if (gfx) gfx->fillRect(x, y, w, h, color); }
    
    void fillArc(int16_t cx, int16_t cy, int16_t r1, int16_t r2, float startAngle, float endAngle, uint16_t color) {
        if (!gfx || r1 == r2) return;
        if (r1 < r2) { int16_t tmp = r1; r1 = r2; r2 = tmp; }
        
        while (endAngle < startAngle) endAngle += 360.0f;
        float sweep = endAngle - startAngle;
        if (sweep <= 0.001f) return;
        if (sweep > 360.0f) sweep = 360.0f;

        float step = 6.0f; // Fast, smooth, direct QSPI triangle mesh arc
        int numSteps = (int)(sweep / step);
        if (numSteps < 1) numSteps = 1;
        float actualStep = sweep / numSteps;

        const float DEG2RAD = 3.14159265358979323846f / 180.0f;

        float rad0 = startAngle * DEG2RAD;
        float sin0 = sinf(rad0);
        float cos0 = cosf(rad0);

        int16_t x0_out = cx + (int16_t)roundf(r1 * sin0);
        int16_t y0_out = cy - (int16_t)roundf(r1 * cos0);
        int16_t x0_in  = cx + (int16_t)roundf(r2 * sin0);
        int16_t y0_in  = cy - (int16_t)roundf(r2 * cos0);

        gfx->startWrite();
        for (int i = 1; i <= numSteps; i++) {
            float a = startAngle + i * actualStep;
            float rad1 = a * DEG2RAD;
            float sin1 = sinf(rad1);
            float cos1 = cosf(rad1);

            int16_t x1_out = cx + (int16_t)roundf(r1 * sin1);
            int16_t y1_out = cy - (int16_t)roundf(r1 * cos1);
            int16_t x1_in  = cx + (int16_t)roundf(r2 * sin1);
            int16_t y1_in  = cy - (int16_t)roundf(r2 * cos1);

            gfx->fillTriangle(x0_out, y0_out, x1_out, y1_out, x0_in, y0_in, color);
            gfx->fillTriangle(x0_in, y0_in, x1_out, y1_out, x1_in, y1_in, color);

            x0_out = x1_out;
            y0_out = y1_out;
            x0_in  = x1_in;
            y0_in  = y1_in;
        }
        gfx->endWrite();
    }
    
    void fillCircle(int16_t x, int16_t y, int16_t r, uint32_t color) { if (gfx) gfx->fillCircle(x, y, r, color); }
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint32_t color) { if (gfx) gfx->drawFastHLine(x, y, w, color); }
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint32_t color) { if (gfx) gfx->drawFastVLine(x, y, h, color); }
    
    int16_t width() { return gfx ? gfx->width() : LCD_WIDTH; }
    int16_t height() { return gfx ? gfx->height() : LCD_HEIGHT; }
    
    void flush() {
        if (gfx) {
            gfx->flush();
        }
    }
    
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
