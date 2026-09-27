#pragma once
#include "IScreen.h"
#include "../UI.h"

template <typename DisplayPolicy = void>
class DisconnectedMsgScreen : public IScreen {
    DisplayPolicy* pDisplay = nullptr;
    AppState* pState = nullptr;
public:
    void setDisplay(DisplayPolicy* d, AppState* s = nullptr) { pDisplay = d; pState = s; }

    void init() override {}
    void show() override {
        if (pDisplay && pState) onShow(*pDisplay, *pState);
    }
    void hide() override {}
    void update(const AppState& state) override {
        pState = const_cast<AppState*>(&state);
        if (pDisplay) onUpdate(*pDisplay, const_cast<AppState&>(state));
    }
    lv_obj_t* getRoot() const override { return nullptr; }
    void destroy() override {}

    void onShow(DisplayPolicy& tft, AppState& state) {
        tft.fillScreen(0x0000); 
        UI<DisplayPolicy> ui(tft);
        ui.begin();
        ui.setCursorY(0.4f);
        
        ui.textCenter("Disconnected", 0xFFFF, 0.12f, 0.0f, 0xF800);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) {}
};
