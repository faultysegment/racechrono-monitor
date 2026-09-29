#pragma once
#include "IScreen.h"
#include "../AppState.h"
#include <lvgl.h>

template <typename DisplayPolicy = void>
class CircularDisconnectedScreen : public IScreen {
    lv_obj_t* root = nullptr;
    lv_obj_t* title_label = nullptr;
    lv_obj_t* spinner = nullptr;

public:
    ~CircularDisconnectedScreen() override { destroy(); }

    void setDisplay(void* d = nullptr, AppState* s = nullptr) {}

    void init() override {
        if (root) return;

        root = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(root, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

        title_label = lv_label_create(root);
        lv_obj_set_style_text_font(title_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(title_label, lv_color_make(255, 0, 0), 0);
        lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(title_label, "Disconnected");
        lv_obj_align(title_label, LV_ALIGN_CENTER, 0, -60);

        spinner = lv_spinner_create(root, 1000, 60);
        lv_obj_set_size(spinner, 90, 90);
        lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 45);
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
