#pragma once

#include "../../AppState.h"
#include "../../View.h"
#include "../../AppLogic.h"
#include "Policies/RealHWPolicy.h"
#include "Policies/RealDisplayPolicy.h"
#include "Policies/RealBLEPolicy.h"
#include "Policies/RealStoragePolicy.h"
#include "Screens/MonitorScreen.h"
#include "Screens/DualMonitorScreen.h"
#include "Screens/DisconnectedMsgScreen.h"
#include "Screens/ConfigMonitorScreen.h"

class App {
    AppState state;
    EventBus eventBus;
    View<RealDisplayPolicy, RealHWPolicy> appView;
    AppLogic<RealBLEPolicy, RealHWPolicy, RealStoragePolicy> appLogic;
    MonitorScreen<RealDisplayPolicy> monitor0{0};
    MonitorScreen<RealDisplayPolicy> monitor1{1};
    DualMonitorScreen<RealDisplayPolicy> dualMonitor;
    DisconnectedMsgScreen<RealDisplayPolicy> disconnectedMsg;
    ConfigMonitorScreen<RealDisplayPolicy> configSpeed{"SPEED LIMIT", &state.speedLimit};
    ConfigMonitorScreen<RealDisplayPolicy> configTime{"TIME LIMIT", &state.timeLimit};

public:
    App() : appView(state), appLogic(state, eventBus) {
        appView.addConnectedScreen(&monitor0);
        appView.addConnectedScreen(&monitor1);
        appView.addConnectedScreen(&dualMonitor);
        
        appView.addDisconnectedScreen(&disconnectedMsg);
        appView.addDisconnectedScreen(&configSpeed);
        appView.addDisconnectedScreen(&configTime);
    }

    void setup() {
        appView.init();
        appLogic.setup();
    }

    void pollInput() {
        appLogic.pollInput();
    }

    void pollLogic() {
        appLogic.pollLogic();
    }

    void processEvent(const Event& e) {
        appView.processEvent(e);
        appLogic.processEvent(e);
    }
    
    void tickUI() {
        appLogic.processEvent(Event{EventType::SYS_TICK, 0, 0, 0});
    }

    bool isRunning() {
        return true;
    }

    EventBus& getEventBus() {
        return eventBus;
    }
};
