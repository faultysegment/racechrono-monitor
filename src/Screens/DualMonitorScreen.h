#pragma once
#include "IScreen.h"
#include "../AppState.h"
#include "../ColorUtils.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

template <typename DisplayPolicy = void>
class DualMonitorScreen : public IScreen {
    ScreenSlotConfig mTopSlot;
    ScreenSlotConfig mBtmSlot;

    lv_obj_t* root = nullptr;
    lv_obj_t* top_title_label = nullptr;
    lv_obj_t* top_value_label = nullptr;
    lv_obj_t* top_bar = nullptr;
    lv_obj_t* btm_title_label = nullptr;
    lv_obj_t* btm_value_label = nullptr;
    lv_obj_t* btm_bar = nullptr;

public:
    DualMonitorScreen(int topIdx = 0, int btmIdx = 1) : mTopSlot(topIdx), mBtmSlot(btmIdx) {}
    DualMonitorScreen(const ScreenSlotConfig& top, const ScreenSlotConfig& btm) : mTopSlot(top), mBtmSlot(btm) {}
    ~DualMonitorScreen() override { destroy(); }

    void setSlots(const ScreenSlotConfig& top, const ScreenSlotConfig& btm) {
        mTopSlot = top;
        mBtmSlot = btm;
    }

    void setMonitors(int topIdx, int btmIdx) {
        mTopSlot.monitorIndex = topIdx;
        mBtmSlot.monitorIndex = btmIdx;
    }

    void setDisplay(void* d = nullptr, AppState* s = nullptr) {}

    void init() override {
        if (root) return;

        root = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(root, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

        // Top Slot
        top_title_label = lv_label_create(root);
        lv_obj_align(top_title_label, LV_ALIGN_TOP_LEFT, 10, 5);
        lv_obj_set_style_text_font(top_title_label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(top_title_label, ColorUtils::color565ToLVGL(mTopSlot.titleColor), 0);
        lv_label_set_text(top_title_label, "WAIT");

        top_value_label = lv_label_create(root);
        lv_obj_align(top_value_label, LV_ALIGN_TOP_MID, 0, 20);
        lv_obj_set_style_text_font(top_value_label, &lv_font_montserrat_32, 0);
        lv_label_set_text(top_value_label, "---");

        top_bar = lv_bar_create(root);
        lv_obj_set_size(top_bar, lv_pct(100), 12);
        lv_obj_align(top_bar, LV_ALIGN_TOP_MID, 0, 65);
        lv_bar_set_range(top_bar, 0, 1000);
        lv_bar_set_value(top_bar, 0, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(top_bar, lv_color_make(60, 60, 60), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(top_bar, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(top_bar, lv_color_make(120, 120, 120), LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(top_bar, LV_OPA_COVER, LV_PART_INDICATOR);

        // Bottom Slot
        btm_title_label = lv_label_create(root);
        lv_obj_align(btm_title_label, LV_ALIGN_TOP_LEFT, 10, 85);
        lv_obj_set_style_text_font(btm_title_label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(btm_title_label, ColorUtils::color565ToLVGL(mBtmSlot.titleColor), 0);
        lv_label_set_text(btm_title_label, "WAIT");

        btm_value_label = lv_label_create(root);
        lv_obj_align(btm_value_label, LV_ALIGN_TOP_MID, 0, 105);
        lv_obj_set_style_text_font(btm_value_label, &lv_font_montserrat_32, 0);
        lv_label_set_text(btm_value_label, "---");

        btm_bar = lv_bar_create(root);
        lv_obj_set_size(btm_bar, lv_pct(100), 12);
        lv_obj_align(btm_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_bar_set_range(btm_bar, 0, 1000);
        lv_bar_set_value(btm_bar, 0, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(btm_bar, lv_color_make(60, 60, 60), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(btm_bar, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btm_bar, lv_color_make(120, 120, 120), LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(btm_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    }

    void show() override {
        if (!root) init();
    }

    void hide() override {}

    void updateSlot(int mIdx, const ScreenSlotConfig& slot, lv_obj_t* titleLbl, lv_obj_t* valLbl, lv_obj_t* barObj, const AppState& state) {
        if (!titleLbl || !valLbl || !barObj) return;

        if (mIdx < 0 || state.nextMonitorId <= mIdx) {
            lv_label_set_text(titleLbl, "WAIT");
            lv_label_set_text(valLbl, "---");
            lv_bar_set_value(barObj, 0, LV_ANIM_OFF);
            return;
        }

        lv_label_set_text(titleLbl, state.monitors[mIdx].title);
        lv_obj_set_style_text_color(titleLbl, ColorUtils::color565ToLVGL(slot.titleColor), 0);

        float* limitPtr = state.monitors[mIdx].limitPtr;
        float currentLimit = (limitPtr && *limitPtr > 0.0001f) ? *limitPtr : 1.0f;

        if (state.monitors[mIdx].hasException) {
            lv_label_set_text(valLbl, "ERR");
            lv_obj_set_style_text_color(valLbl, ColorUtils::color565ToLVGL(0xF800), 0);
            lv_bar_set_value(barObj, 0, LV_ANIM_OFF);
        } else if (state.monitors[mIdx].value != AppState::INVALID_VALUE) {
            float val = (float)state.monitors[mIdx].value * state.monitors[mIdx].multiplier;
            uint16_t c565 = 0x7BEF;
            if (val > 0) c565 = slot.positiveColor;
            else if (val < 0) c565 = slot.negativeColor;
            lv_color_t color = ColorUtils::color565ToLVGL(c565);

            char valBuf[32];
            if (state.monitors[mIdx].decimals == 2) {
                snprintf(valBuf, sizeof(valBuf), "%+.2f", val);
            } else {
                snprintf(valBuf, sizeof(valBuf), "%+.1f", val);
            }
            lv_label_set_text(valLbl, valBuf);
            lv_obj_set_style_text_color(valLbl, color, 0);

            float pct = std::min(1.0f, std::abs(val) / currentLimit);
            lv_bar_set_value(barObj, (int)(pct * 1000.0f), LV_ANIM_OFF);
            lv_obj_set_style_bg_color(barObj, color, LV_PART_INDICATOR);
        } else {
            lv_label_set_text(valLbl, state.monitors[mIdx].decimals == 2 ? "--.--" : "--.-");
            lv_obj_set_style_text_color(valLbl, ColorUtils::color565ToLVGL(0x7BEF), 0);
            lv_bar_set_value(barObj, 0, LV_ANIM_OFF);
        }
    }

    void update(const AppState& state) override {
        if (!root) init();
        updateSlot(mTopSlot.monitorIndex, mTopSlot, top_title_label, top_value_label, top_bar, state);
        updateSlot(mBtmSlot.monitorIndex, mBtmSlot, btm_title_label, btm_value_label, btm_bar, state);
    }

    lv_obj_t* getRoot() const override { return root; }
    lv_obj_t* getTopTitleLabel() const { return top_title_label; }
    lv_obj_t* getTopValueLabel() const { return top_value_label; }
    lv_obj_t* getTopBar() const { return top_bar; }
    lv_obj_t* getBtmTitleLabel() const { return btm_title_label; }
    lv_obj_t* getBtmValueLabel() const { return btm_value_label; }
    lv_obj_t* getBtmBar() const { return btm_bar; }

    void destroy() override {
        if (root) {
            lv_obj_del(root);
            root = nullptr;
            top_title_label = nullptr;
            top_value_label = nullptr;
            top_bar = nullptr;
            btm_title_label = nullptr;
            btm_value_label = nullptr;
            btm_bar = nullptr;
        }
    }

    template <typename DP>
    void onShow(DP& tft, AppState& state) {}
    template <typename DP>
    void onUpdate(DP& tft, AppState& state) {}
};
