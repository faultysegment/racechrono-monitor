#pragma once
#include "IScreen.h"
#include "../CircularUI.h"

template <typename DisplayPolicy = void>
class CircularConfiguringScreen : public IScreen {
public:
    void init() override {}
    void show() override {}
    void hide() override {}
    void update(const AppState& state) override {}
    lv_obj_t* getRoot() const override { return nullptr; }
    void destroy() override {}

    void onShow(DisplayPolicy& tft, AppState& state) {
        tft.fillScreen(0x0000);
        CircularUI<DisplayPolicy> ui(tft);
        ui.textCenter("CONFIG MODE", 0x07FF, 0.09f, 0.35f);
        ui.textCenter("Editing...", 0xFFFF, 0.07f, 0.60f);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) {}
};
