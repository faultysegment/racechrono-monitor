#pragma once
#include "IScreen.h"
#include "../CircularUI.h"

template <typename DisplayPolicy>
class CircularDisconnectedScreen : public IScreen<DisplayPolicy> {
public:
    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.fillScreen(0x0000); 
        CircularUI<DisplayPolicy> ui(tft);
        ui.begin();
        
        // Draw red accent ring indicating disconnected state
        int cx = tft.width() / 2;
        int cy = tft.height() / 2;
        int radiusOut = std::min(cx, cy) - 5;
        int radiusIn = radiusOut - ui.h(0.025f);
        tft.fillArc(cx, cy, radiusOut, radiusIn, 0.0f, 360.0f, 0xF800);
        
        // Main status text
        ui.textCenter("DISCONNECTED", 0xF800, 0.085f, 0.38f);
        
        // Subtitle
        ui.textCenter("Waiting for BLE...", 0xFFFF, 0.055f, 0.49f);
        
        // Bottom hint
        ui.textCenter("Swipe to configure", 0x7BEF, 0.045f, 0.65f);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {}
};
