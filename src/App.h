#pragma once

#include "AppState.h"
#include "View.h"
#include "AppLogic.h"
#include "Screens/MonitorScreen.h"
#include "Screens/DualMonitorScreen.h"
#include "Screens/DisconnectedMsgScreen.h"
#include "EventBus.h"

template <typename DisplayPolicy, typename HWPolicy, typename BLEPolicy, typename StoragePolicy, typename ViewPolicy>
class App {
    AppState state;
    EventBus eventBus;
    HWPolicy hwPolicy;
    StoragePolicy storagePolicy;
    View<DisplayPolicy, HWPolicy> appView;
    AppLogic<BLEPolicy, HWPolicy, StoragePolicy> appLogic;
    ViewPolicy viewPolicy;
    
public:
    App() : appView(state, hwPolicy), appLogic(state, eventBus, hwPolicy, storagePolicy), viewPolicy(state) {}

    void setup() {
        hwPolicy.initBoard();
        appView.init();
        appLogic.setup();
        viewPolicy.setupScreens(appView, state);
        if (state.currentScreenIndex >= state.numConnectedScreens) {
            state.currentScreenIndex = 0;
        }
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

    AppState& getState() {
        return state;
    }

    StoragePolicy& getStorage() {
        return storagePolicy;
    }

    HWPolicy& getHW() {
        return hwPolicy;
    }
};
