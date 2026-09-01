#pragma once
#include "IScreen.h"
#include "../CircularUI.h"

template <typename DisplayPolicy>
class CircularConfiguringScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000);
        CircularUI<DisplayPolicy> ui(tft);
        ui.textCenter("CONFIG MODE", 0x07FF, 0.15f, 0.35f);
        ui.textCenter("Editing...", 0xFFFF, 0.10f, 0.60f);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        onShow(tft, state);
    }
};
