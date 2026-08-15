#pragma once
#include "IScreen.h"

template <typename DisplayPolicy>
class HudScreenWrapper : public IScreen<DisplayPolicy> {
private:
    IScreen<DisplayPolicy>* innerScreen;

public:
    explicit HudScreenWrapper(IScreen<DisplayPolicy>* screen = nullptr) : innerScreen(screen) {}

    void onShow(DisplayPolicy& tft, AppState& state) override {
        tft.setHudMode(true);
        if (innerScreen) {
            innerScreen->onShow(tft, state);
        }
    }

    void onUpdate(DisplayPolicy& tft, AppState& state) override {
        tft.setHudMode(true);
        if (innerScreen) {
            innerScreen->onUpdate(tft, state);
        }
    }
};
