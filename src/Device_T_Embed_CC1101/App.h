#pragma once

#include "../../Model.h"
#include "../../View.h"
#include "../../Controller.h"
#include "Policies/RealHWPolicy.h"
#include "Policies/RealDisplayPolicy.h"
#include "Policies/RealBLEPolicy.h"
#include "Policies/RealStoragePolicy.h"
#include "Screens/MonitorScreen.h"
#include "Screens/DualMonitorScreen.h"
#include "Screens/DisconnectedMsgScreen.h"
#include "Screens/ConfigMonitorScreen.h"

class App {
    Model model;
    View<RealDisplayPolicy, RealHWPolicy> appView;
    Controller<Model, View<RealDisplayPolicy, RealHWPolicy>, RealBLEPolicy, RealHWPolicy, RealStoragePolicy> appController;
    MonitorScreen<RealDisplayPolicy> monitor0{0};
    MonitorScreen<RealDisplayPolicy> monitor1{1};
    DualMonitorScreen<RealDisplayPolicy> dualMonitor;
    DisconnectedMsgScreen<RealDisplayPolicy> disconnectedMsg;
    ConfigMonitorScreen<RealDisplayPolicy> configSpeed{"SPEED LIMIT", &model.speedLimit};
    ConfigMonitorScreen<RealDisplayPolicy> configTime{"TIME LIMIT", &model.timeLimit};

public:
    App() : appView(model), appController(model, appView) {
        appView.addConnectedScreen(&monitor0);
        appView.addConnectedScreen(&monitor1);
        appView.addConnectedScreen(&dualMonitor);
        
        appView.addDisconnectedScreen(&disconnectedMsg);
        appView.addDisconnectedScreen(&configSpeed);
        appView.addDisconnectedScreen(&configTime);
    }

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
