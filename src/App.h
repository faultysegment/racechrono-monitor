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
        // eink style: we no longer spam UI_UPDATE here.
        // Screen only updates when explicit events push UI_UPDATE to the bus.
    }

    EventBus& getEventBus() {
        return eventBus;
    }
};
