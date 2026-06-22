#pragma once
#include "IScreen.h"
#include "../UI.h"

template <typename DisplayPolicy>
class DisconnectedMsgScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        UI<DisplayPolicy> ui(tft);
        ui.begin();
        ui.setCursorY(0.4f);
        
        ui.textCenter("Disconnected", 0xFFFF, 0.12f, 0.0f, 0xF800);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {}
};
