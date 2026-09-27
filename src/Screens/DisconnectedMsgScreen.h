#pragma once
#include "IScreen.h"
#include "../AppState.h"
#include <lvgl.h>

template <typename DisplayPolicy = void>
class DisconnectedMsgScreen : public IScreen {
    lv_obj_t* root = nullptr;
    lv_obj_t* title_label = nullptr;
    lv_obj_t* spinner = nullptr;

public:
    ~DisconnectedMsgScreen() override { destroy(); }

    void setDisplay(void* d = nullptr, AppState* s = nullptr) {}

    void init() override {
        if (root) return;

        root = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(root, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

        title_label = lv_label_create(root);
        lv_obj_align(title_label, LV_ALIGN_CENTER, 0, -35);
        lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(title_label, lv_color_make(255, 0, 0), 0);
        lv_label_set_text(title_label, "Disconnected");

        spinner = lv_spinner_create(root, 1000, 60);
        lv_obj_set_size(spinner, 50, 50);
        lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 30);
        lv_obj_set_style_arc_color(spinner, lv_color_make(255, 0, 0), LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(spinner, lv_color_make(50, 50, 50), LV_PART_MAIN);
    }

    void show() override {
        if (!root) init();
    }

    void hide() override {}

    void update(const AppState& state) override {
        if (!root) init();
    }

    lv_obj_t* getRoot() const override { return root; }
    lv_obj_t* getTitleLabel() const { return title_label; }
    lv_obj_t* getSpinner() const { return spinner; }

    void destroy() override {
        if (root) {
            lv_obj_del(root);
            root = nullptr;
            title_label = nullptr;
            spinner = nullptr;
        }
    }

    template <typename DP>
    void onShow(DP& tft, AppState& state) {}
    template <typename DP>
    void onUpdate(DP& tft, AppState& state) {}
};
