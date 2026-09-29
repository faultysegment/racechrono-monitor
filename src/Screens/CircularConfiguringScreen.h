#pragma once
#include "IScreen.h"
#include "../AppState.h"
#include <lvgl.h>

template <typename DisplayPolicy = void>
class CircularConfiguringScreen : public IScreen {
    lv_obj_t* root = nullptr;
    lv_obj_t* title_label = nullptr;
    lv_obj_t* desc_label = nullptr;

public:
    ~CircularConfiguringScreen() override { destroy(); }

    void setDisplay(void* d = nullptr, AppState* s = nullptr) {}

    void init() override {
        if (root) return;

        root = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(root, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

        title_label = lv_label_create(root);
        lv_obj_set_style_text_font(title_label, &lv_font_montserrat_32, 0);
        lv_obj_set_style_text_color(title_label, lv_color_make(0, 255, 255), 0); // Cyan (0x07FF)
        lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(title_label, "CONFIG MODE");
        lv_obj_align(title_label, LV_ALIGN_CENTER, 0, -45);

        desc_label = lv_label_create(root);
        lv_obj_set_style_text_font(desc_label, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(desc_label, lv_color_white(), 0);
        lv_obj_set_style_text_align(desc_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(desc_label, "Editing...");
        lv_obj_align(desc_label, LV_ALIGN_CENTER, 0, 35);
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
    lv_obj_t* getDescLabel() const { return desc_label; }

    void destroy() override {
        if (root) {
            lv_obj_del(root);
            root = nullptr;
            title_label = nullptr;
            desc_label = nullptr;
        }
    }

    template <typename DP>
    void onShow(DP& tft, AppState& state) {}
    template <typename DP>
    void onUpdate(DP& tft, AppState& state) {}
};
