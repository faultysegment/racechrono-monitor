#pragma once
#include "IScreen.h"
#include "../CircularUI.h"

template <typename DisplayPolicy = void>
class CircularDisconnectedScreen : public IScreen {
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
        ui.textCenter("DISCONNECTED", 0xF800, 0.08f, 0.48f);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) {}
};
