#pragma once
#include "../../AppState.h"

template <typename DisplayPolicy>
class IScreen {
public:
    virtual ~IScreen() {}
    
    // Called once when the screen becomes active
    virtual void onShow(DisplayPolicy& tft, AppState& state) = 0;
    
    // Called periodically to update dynamic data
    virtual void onUpdate(DisplayPolicy& tft, AppState& state) = 0;
};


