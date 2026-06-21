#pragma once
#include "AppState.h"
#include "View.h"
#include "Screens/MonitorScreen.h"
#include "Screens/DualMonitorScreen.h"
#include "Screens/DisconnectedMsgScreen.h"
#include "Screens/ConfigMonitorScreen.h"

template <typename DisplayPolicy>
class NativeViewPolicy {
    MonitorScreen<DisplayPolicy> monitor0{0};
    MonitorScreen<DisplayPolicy> monitor1{1};
    DualMonitorScreen<DisplayPolicy> dualMonitor;
    DisconnectedMsgScreen<DisplayPolicy> disconnectedMsg;
    ConfigMonitorScreen<DisplayPolicy> configSpeed;
    ConfigMonitorScreen<DisplayPolicy> configTime;

public:
    NativeViewPolicy(AppState& state) 
        : configSpeed("SPEED LIMIT", &state.speedLimit),
          configTime("TIME LIMIT", &state.timeLimit) {}

    template <typename HWPolicy>
    void setupScreens(View<DisplayPolicy, HWPolicy>& appView) {
        appView.addConnectedScreen(&monitor0);
        appView.addConnectedScreen(&monitor1);
        appView.addConnectedScreen(&dualMonitor);
        
        appView.addDisconnectedScreen(&disconnectedMsg);
        appView.addDisconnectedScreen(&configSpeed);
        appView.addDisconnectedScreen(&configTime);
    }
};
