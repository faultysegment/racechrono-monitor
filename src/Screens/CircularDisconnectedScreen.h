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
        tft.fillCircle(cx, cy, radiusOut, 0xF800);
        tft.fillCircle(cx, cy, radiusIn, 0x0000);
        
        // Main status text (Big bold red, size 4)
        ui.textCenter("DISCONNECTED", 0xF800, 0.14f, 0.38f);
        
        // Subtitle (Crisp white, size 3)
        ui.textCenter("Waiting for BLE...", 0xFFFF, 0.10f, 0.50f);
        
        // Bottom hint (Dimmed gray, size 2)
        ui.textCenter("Swipe to configure", 0x7BEF, 0.07f, 0.65f);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {}
};
