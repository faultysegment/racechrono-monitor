#pragma once
#include "IScreen.h"
#include "../UI.h"

template <typename DisplayPolicy>
class ConfiguringScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000);
        UI<DisplayPolicy> ui(tft);
        ui.begin();
        ui.setCursorY(0.25f);
        ui.textCenter("CONFIG MODE", 0x07FF, 0.18f); // Cyan
        ui.setCursorY(0.55f);
        ui.textCenter("Editing configuration...", 0xFFFF, 0.12f);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        onShow(tft, state);
    }
};
