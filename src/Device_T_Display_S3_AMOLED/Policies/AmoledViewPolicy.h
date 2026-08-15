#pragma once
#include "../../AppState.h"
#include "../../View.h"
#include "../../Screens/CircularMonitorScreen.h"
#include "../../Screens/CircularDualMonitorScreen.h"
#include "../../Screens/DisconnectedMsgScreen.h"
#include "../../Screens/CircularConfigMonitorScreen.h"
#include "../../Screens/HudScreenWrapper.h"

template <typename DisplayPolicy>
class AmoledViewPolicy {
    CircularMonitorScreen<DisplayPolicy> monitor0{0};
    CircularMonitorScreen<DisplayPolicy> monitor1{1};
    CircularDualMonitorScreen<DisplayPolicy> dualMonitor;

    HudScreenWrapper<DisplayPolicy> hudMonitor0{&monitor0};
    HudScreenWrapper<DisplayPolicy> hudMonitor1{&monitor1};
    HudScreenWrapper<DisplayPolicy> hudDualMonitor{&dualMonitor};

    DisconnectedMsgScreen<DisplayPolicy> disconnectedMsg;
    CircularConfigMonitorScreen<DisplayPolicy> configSpeed;
    CircularConfigMonitorScreen<DisplayPolicy> configTime;

    HudScreenWrapper<DisplayPolicy> hudDisconnectedMsg{&disconnectedMsg};
    HudScreenWrapper<DisplayPolicy> hudConfigSpeed{&configSpeed};
    HudScreenWrapper<DisplayPolicy> hudConfigTime{&configTime};

public:
    AmoledViewPolicy(AppState& state) 
        : configSpeed(1),
          configTime(2) {}

    template <typename HWPolicy>
    void setupScreens(View<DisplayPolicy, HWPolicy>& appView) {
        appView.addConnectedScreen(&monitor0);
        appView.addConnectedScreen(&monitor1);
        appView.addConnectedScreen(&dualMonitor);
        appView.addConnectedScreen(&hudMonitor0);
        appView.addConnectedScreen(&hudMonitor1);
        appView.addConnectedScreen(&hudDualMonitor);
        
        appView.addDisconnectedScreen(&disconnectedMsg);
        appView.addDisconnectedScreen(&configSpeed);
        appView.addDisconnectedScreen(&configTime);
        appView.addDisconnectedScreen(&hudDisconnectedMsg);
        appView.addDisconnectedScreen(&hudConfigSpeed);
        appView.addDisconnectedScreen(&hudConfigTime);
    }
};
