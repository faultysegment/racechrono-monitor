#pragma once
#include <lvgl.h>

/**
 * @brief Utility for rounding LVGL refresh areas to satisfy hardware controller constraints.
 * 
 * AMOLED controllers such as Chipone CO5300, Sino Wealth SH8601, and Raydium RM67162
 * over QSPI interfaces require even column/row addresses and even window dimensions (minimum 2x2).
 */
struct DisplayRounder {
    static void roundToEven(lv_disp_drv_t* drv, lv_area_t* area) {
        if (!area) return;

        if (area->x1 < 0) area->x1 = 0;
        if (area->y1 < 0) area->y1 = 0;

        // Force start coordinates to even boundary
        area->x1 &= ~1;
        area->y1 &= ~1;

        // Force end coordinates to odd index so (end - start + 1) is even width/height
        area->x2 |= 1;
        area->y2 |= 1;

        if (drv) {
            if (drv->hor_res > 0 && area->x2 >= drv->hor_res) {
                area->x2 = drv->hor_res - 1;
            }
            if (drv->ver_res > 0 && area->y2 >= drv->ver_res) {
                area->y2 = drv->ver_res - 1;
            }
        }
    }
};
