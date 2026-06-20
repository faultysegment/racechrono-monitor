#pragma once
#include "IScreen.h"

template <typename DisplayPolicy>
class DisconnectedMsgScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        tft.setTextColor(0xFFFF, 0xF800); 
        tft.setTextSize(6); // Larger size for HD
        
        int screenW = tft.width();
        int screenH = tft.height();
        tft.setCursor(screenW / 2 - 350, screenH / 2 - 50);
        tft.print("No BLE connection!");
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {}
};



