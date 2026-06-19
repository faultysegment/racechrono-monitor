#include <Arduino.h>
#include "Model.h"
#include "View.h"
#include "Controller.h"

#include "Policies/RealHWPolicy.h"
#include "Policies/RealDisplayPolicy.h"
#include "Policies/RealBLEPolicy.h"
#include "Policies/RealStoragePolicy.h"

// Define concrete types
using AppView = View<RealDisplayPolicy, RealHWPolicy>;
using AppController = Controller<Model, AppView, RealBLEPolicy, RealHWPolicy, RealStoragePolicy>;

Model model;
AppView view(model);
AppController controller(model, view);

void setup() {
    controller.setup();
}

void loop() {
    controller.loop();
}