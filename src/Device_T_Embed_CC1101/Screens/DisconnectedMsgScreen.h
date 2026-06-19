#pragma once
#include "IScreen.h"

template <typename DisplayPolicy>
class DisconnectedMsgScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, Model& model) override {
        tft.fillScreen(0x0000); 
        tft.setTextColor(0xFFFF, 0xF800); 
        tft.setTextSize(2);
        
        int screenW = tft.width();
        tft.setCursor(screenW / 2 - 100, 40);
        tft.print("No BLE connection!");
    }

    void onUpdate(DisplayPolicy& tft, Model& model) override {}
};
