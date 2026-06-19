#pragma once

#include <SDL2/SDL.h>
#include "../../Model.h"
#include "../../View.h"
#include "../../Controller.h"
#include "Policies/RealHWPolicy.h"
#include "Policies/RealDisplayPolicy.h"
#include "Policies/RealBLEPolicy.h"
#include "Policies/RealStoragePolicy.h"

class App {
    Model model;
    View<RealDisplayPolicy, RealHWPolicy> appView;
    Controller<Model, View<RealDisplayPolicy, RealHWPolicy>, RealBLEPolicy, RealHWPolicy, RealStoragePolicy> appController;
    bool running = true;

public:
    App() : appView(model), appController(model, appView) {}

    void setup() {
        appController.setup();
    }

    void loop() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        appController.loop();
    }

    bool isRunning() {
        return running;
    }
};
