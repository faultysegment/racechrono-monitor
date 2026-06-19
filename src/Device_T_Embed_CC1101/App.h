#pragma once

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

public:
    App() : appView(model), appController(model, appView) {}

    void setup() {
        appController.setup();
    }

    void loop() {
        appController.loop();
    }

    bool isRunning() {
        return true;
    }
};
