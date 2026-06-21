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
        
        // Use textCenter but we want red background
        tft.setTextColor(0xFFFF, 0xF800); 
        tft.setTextSize(ui.textSize(0.12f));
        int nudgeLeft = ui.w(0.25f);
        tft.setCursor(ui.w(0.5f) - nudgeLeft, ui.getCursorY());
        tft.print("No BLE connection!");
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {}
};
