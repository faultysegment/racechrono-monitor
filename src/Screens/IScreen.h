#pragma once
#include <lvgl.h>
#include "../AppState.h"

class IScreen {
public:
    virtual ~IScreen() = default;

    // Creates the screen root object and child widgets
    virtual void init() = 0;

    // Makes this screen active / visible in LVGL
    virtual void show() = 0;

    // Hides or deactivates this screen
    virtual void hide() = 0;

    // Updates widget states (labels, bars, arcs, colors) from AppState
    virtual void update(const AppState& state) = 0;

    // Returns the root lv_obj_t* container/screen
    virtual lv_obj_t* getRoot() const = 0;

    // Cleans up the widget tree
    virtual void destroy() = 0;
};
