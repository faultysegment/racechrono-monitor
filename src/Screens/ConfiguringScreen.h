#pragma once
#include "IScreen.h"
#include "../UI.h"

template <typename DisplayPolicy = void>
class ConfiguringScreen : public IScreen {
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
        ui.setCursorY(0.25f);
        ui.textCenter("CONFIG MODE", 0x07FF, 0.18f); // Cyan
        ui.setCursorY(0.55f);
        ui.textCenter("Editing configuration...", 0xFFFF, 0.12f);
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) {
        onShow(tft, state);
    }
};
