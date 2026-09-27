#pragma once
#include "IScreen.h"
#include "../AppState.h"
#include "../ColorUtils.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

template <typename DisplayPolicy = void>
class MonitorScreen : public IScreen {
    ScreenSlotConfig mSlot;

    lv_obj_t* root = nullptr;
    lv_obj_t* title_label = nullptr;
    lv_obj_t* battery_label = nullptr;
    lv_obj_t* value_label = nullptr;
    lv_obj_t* bar = nullptr;

public:
    MonitorScreen(int monitorIndex = 0) : mSlot(monitorIndex) {}
    MonitorScreen(const ScreenSlotConfig& slot) : mSlot(slot) {}
    ~MonitorScreen() override {
        destroy();
    }

    void setConfig(const ScreenSlotConfig& slot) {
        mSlot = slot;
    }

    void setMonitorIndex(int idx) {
        mSlot.monitorIndex = idx;
    }

    void setDisplay(void* d = nullptr, AppState* s = nullptr) {}

    void init() override {
        if (root) return;

        root = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(root, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

        title_label = lv_label_create(root);
        lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 10, 8);
        lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(title_label, ColorUtils::color565ToLVGL(mSlot.titleColor), 0);
        lv_label_set_text(title_label, "WAIT");

        battery_label = lv_label_create(root);
        lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -10, 8);
        lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(battery_label, lv_color_white(), 0);
        lv_label_set_text(battery_label, "---%");

        value_label = lv_label_create(root);
        lv_obj_align(value_label, LV_ALIGN_CENTER, 0, -10);
        lv_obj_set_style_text_font(value_label, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(value_label, lv_color_make(120, 120, 120), 0);
        lv_label_set_text(value_label, "---");

        bar = lv_bar_create(root);
        lv_obj_set_size(bar, lv_pct(100), 24);
        lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_bar_set_range(bar, 0, 1000);
        lv_bar_set_value(bar, 0, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar, lv_color_make(60, 60, 60), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar, lv_color_make(120, 120, 120), LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);
    }

    void show() override {
        if (!root) init();
    }

    void hide() override {}

    void update(const AppState& state) override {
        if (!root) init();

        // Battery label
        if (battery_label) {
            char batBuf[16];
            if (state.batteryPercent >= 0 && state.batteryPercent <= 100) {
                snprintf(batBuf, sizeof(batBuf), "%3d%%", state.batteryPercent);
            } else {
                snprintf(batBuf, sizeof(batBuf), "---%%");
            }
            lv_label_set_text(battery_label, batBuf);
        }

        int mIdx = mSlot.monitorIndex;
        if (mIdx < 0 || state.nextMonitorId <= mIdx) {
            lv_label_set_text(title_label, "WAIT");
            lv_label_set_text(value_label, "---");
            lv_bar_set_value(bar, 0, LV_ANIM_OFF);
            return;
        }

        // Title
        lv_label_set_text(title_label, state.monitors[mIdx].title);
        lv_obj_set_style_text_color(title_label, ColorUtils::color565ToLVGL(mSlot.titleColor), 0);

        float* limitPtr = state.monitors[mIdx].limitPtr;
        float currentLimit = (limitPtr && *limitPtr > 0.0001f) ? *limitPtr : 1.0f;

        if (state.monitors[mIdx].hasException) {
            lv_label_set_text(value_label, "ERR");
            lv_obj_set_style_text_color(value_label, ColorUtils::color565ToLVGL(0xF800), 0);
            lv_bar_set_value(bar, 0, LV_ANIM_OFF);
        } else if (state.monitors[mIdx].value != AppState::INVALID_VALUE) {
            float val = (float)state.monitors[mIdx].value * state.monitors[mIdx].multiplier;
            uint16_t c565 = 0x7BEF;
            if (val > 0) {
                c565 = mSlot.positiveColor;
            } else if (val < 0) {
                c565 = mSlot.negativeColor;
            }
            lv_color_t color = ColorUtils::color565ToLVGL(c565);

            char valBuf[32];
            if (state.monitors[mIdx].decimals == 2) {
                snprintf(valBuf, sizeof(valBuf), "%+.2f", val);
            } else {
                snprintf(valBuf, sizeof(valBuf), "%+.1f", val);
            }
            lv_label_set_text(value_label, valBuf);
            lv_obj_set_style_text_color(value_label, color, 0);

            float pct = std::min(1.0f, std::abs(val) / currentLimit);
            lv_bar_set_value(bar, (int)(pct * 1000.0f), LV_ANIM_OFF);
            lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);
        } else {
            if (state.monitors[mIdx].decimals == 2) {
                lv_label_set_text(value_label, "--.--");
            } else {
                lv_label_set_text(value_label, "--.-");
            }
            lv_obj_set_style_text_color(value_label, ColorUtils::color565ToLVGL(0x7BEF), 0);
            lv_bar_set_value(bar, 0, LV_ANIM_OFF);
        }
    }

    lv_obj_t* getRoot() const override { return root; }
    lv_obj_t* getTitleLabel() const { return title_label; }
    lv_obj_t* getValueLabel() const { return value_label; }
    lv_obj_t* getBatteryLabel() const { return battery_label; }
    lv_obj_t* getBar() const { return bar; }

    void destroy() override {
        if (root) {
            lv_obj_del(root);
            root = nullptr;
            title_label = nullptr;
            battery_label = nullptr;
            value_label = nullptr;
            bar = nullptr;
        }
    }

    template <typename DP>
    void onShow(DP& tft, AppState& state) {}
    template <typename DP>
    void onUpdate(DP& tft, AppState& state) {}
};
