#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class RealDisplayPolicy {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    int cursorX = 0;
    int cursorY = 0;
    uint16_t textColor = 0xFFFF;
    uint16_t bgColor = 0x0000;
    int textSize = 1;

    bool isHudMode = false;

    void setSDLColor(uint16_t color) {
        uint8_t r = (color >> 11) * 8;
        uint8_t g = ((color >> 5) & 0x3F) * 4;
        uint8_t b = (color & 0x1F) * 8;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    }

public:
    void init() {
        SDL_Init(SDL_INIT_VIDEO);
        TTF_Init();
        window = SDL_CreateWindow("RaceChrono Monitor (AMOLED Simulator)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 466, 466, 0);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        // Load default font. Note: requires a ttf file in the working directory!
        font = TTF_OpenFont("font.ttf", 16); 
    }

    void setRotation(int r) { } // Ignore for mock
    void setHudMode(bool hud) { isHudMode = hud; }

    int width() { return 466; }
    int height() { return 466; }

    void fillScreen(uint16_t color) {
        setSDLColor(color);
        SDL_RenderClear(renderer);
    }

    void fillRect(int x, int y, int w, int h, uint16_t color) {
        setSDLColor(color);
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderFillRect(renderer, &rect);
    }

    void fillCircle(int cx, int cy, int r, uint16_t color) {
        setSDLColor(color);
        for (int w = 0; w < r * 2; w++) {
            for (int h = 0; h < r * 2; h++) {
                int dx = r - w; // horizontal offset
                int dy = r - h; // vertical offset
                if ((dx*dx + dy*dy) <= (r * r)) {
                    SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
                }
            }
        }
    }

    void fillArc(int x, int y, int r1, int r2, float startAngle, float endAngle, uint16_t color) {
        setSDLColor(color);
        // angles are in degrees. 0 = top, 90 = right.
        float startRad = (startAngle - 90.0f) * M_PI / 180.0f;
        float endRad = (endAngle - 90.0f) * M_PI / 180.0f;
        
        float step = 1.0f / (float)r1; // approx 1 pixel arc length
        for (float a = startRad; a <= endRad; a += step) {
            float ca = cos(a);
            float sa = sin(a);
            int x1 = x + ca * r2;
            int y1 = y + sa * r2;
            int x2 = x + ca * r1;
            int y2 = y + sa * r1;
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }

    void setCursor(int x, int y) {
        cursorX = x;
        cursorY = y;
    }

    void setTextWrap(bool wrap) {}

    void setTextSize(int size) {
        if (size <= 0) size = 1;
        if (textSize != size) {
            textSize = size;
            if (font) TTF_CloseFont(font);
            font = TTF_OpenFont("font.ttf", 16 * size);
        }
    }

    void setTextColor(uint16_t color, uint16_t bg = 0x0000) {
        textColor = color;
        bgColor = bg;
    }

    void print(const std::string& text) {
        if (!font || text.empty()) return;
        
        uint8_t r = (textColor >> 11) * 8;
        uint8_t g = ((textColor >> 5) & 0x3F) * 4;
        uint8_t b = (textColor & 0x1F) * 8;
        SDL_Color fg = {r, g, b, 255};

        SDL_Surface* surface = nullptr;
        if (textColor != bgColor) {
            uint8_t br = (bgColor >> 11) * 8;
            uint8_t bg_g = ((bgColor >> 5) & 0x3F) * 4;
            uint8_t bb = (bgColor & 0x1F) * 8;
            SDL_Color bg_c = {br, bg_g, bb, 255};
            surface = TTF_RenderText_Shaded(font, text.c_str(), fg, bg_c);
        } else {
            surface = TTF_RenderText_Solid(font, text.c_str(), fg);
        }
        
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            
            // Use native surface size (already scaled by point size)
            int w = surface->w;
            int h = surface->h;
            
            SDL_Rect dest = {cursorX, cursorY, w, h};
            SDL_RenderCopy(renderer, texture, NULL, &dest);
            
            cursorX += w;
            
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);
        }
    }

    void println(const std::string& text) {
        print(text);
        cursorX = 0;
        cursorY += 16 * textSize; // Approximate newline height
    }

    void print(int val) {
        print(std::to_string(val));
    }

    int16_t textWidth(const char* str) {
        if (!font || !str || str[0] == '\0') return 0;
        int w, h;
        if (TTF_SizeText(font, str, &w, &h) == 0) {
            return w;
        }
        return 0;
    }

    void setBacklight(bool on) {
        // Mock backlight by changing brightness or just ignore
    }

    void flush() {
        SDL_RenderPresent(renderer);
    }

    void drawBattery(int percent, bool force = false) {
        static int lastBat = -2;
        if (force || lastBat != percent) {
            lastBat = percent;
            setTextSize(4);
            setTextColor(0xFFFF, 0x0000); 
            setCursor(width() - 250, 20); 
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
