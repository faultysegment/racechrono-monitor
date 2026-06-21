#pragma once

#include "AppState.h"
#include "View.h"
#include "AppLogic.h"
#include "Screens/MonitorScreen.h"
#include "Screens/DualMonitorScreen.h"
#include "Screens/DisconnectedMsgScreen.h"
#include "Screens/ConfigMonitorScreen.h"
#include "EventBus.h"

template <typename DisplayPolicy, typename HWPolicy, typename BLEPolicy, typename StoragePolicy, typename ViewPolicy>
class App {
    AppState state;
    EventBus eventBus;
    View<DisplayPolicy, HWPolicy> appView;
    AppLogic<BLEPolicy, HWPolicy, StoragePolicy> appLogic;
    ViewPolicy viewPolicy;
    
public:
    App() : appView(state), appLogic(state, eventBus), viewPolicy(state) {
        viewPolicy.setupScreens(appView);
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
        appView.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    }

    EventBus& getEventBus() {
        return eventBus;
    }
};
