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

    bool running;

public:
    App() : appView(state), appLogic(state, eventBus), running(true) {
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

    void loop() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        appLogic.pollInput();
        appLogic.pollLogic();

        Event e;
        while (eventBus.try_pop(e)) {
            appView.processEvent(e);
            appLogic.processEvent(e);
        }
        appView.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
        appLogic.processEvent(Event{EventType::SYS_TICK, 0, 0, 0});
        
        SDL_Delay(25);
    }

    bool isRunning() {
        return running;
    }
};
