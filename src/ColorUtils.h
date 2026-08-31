#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace ColorUtils {

inline uint16_t parseHexColor565(const char* hexStr, uint16_t defaultColor) {
    if (!hexStr || hexStr[0] == '\0') {
        return defaultColor;
    }

    const char* ptr = hexStr;
    if (ptr[0] == '#') {
        ptr++;
    } else if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) {
        ptr += 2;
    }

    size_t len = strlen(ptr);
    if (len == 6) {
        char* end = nullptr;
        unsigned long rgb = strtoul(ptr, &end, 16);
        if (end == ptr + len) {
            uint8_t r = (rgb >> 16) & 0xFF;
            uint8_t g = (rgb >> 8) & 0xFF;
            uint8_t b = rgb & 0xFF;
            return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
        }
    } else if (len == 4) { // Direct 16-bit RGB565 hex like "F800"
        char* end = nullptr;
        unsigned long val = strtoul(ptr, &end, 16);
        if (end == ptr + len) {
            return (uint16_t)val;
        }
    }

    return defaultColor;
}

} // namespace ColorUtils
