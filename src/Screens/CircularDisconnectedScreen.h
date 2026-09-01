#pragma once
#include "IScreen.h"
#include "../CircularUI.h"

template <typename DisplayPolicy>
class CircularDisconnectedScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        CircularUI<DisplayPolicy> ui(tft);
        ui.textCenter("DISCONNECTED", 0xF800, 0.08f, 0.48f);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {}
};
