#pragma once
#include "AppState.h"
#include "View.h"
#include "Screens/MonitorScreen.h"
#include "Screens/DualMonitorScreen.h"
#include "Screens/CircularMonitorScreen.h"
#include "Screens/DisconnectedMsgScreen.h"

template <typename DisplayPolicy>
class NativeViewPolicy {
    CircularMonitorScreen<DisplayPolicy> circMonitor0{0};
    CircularMonitorScreen<DisplayPolicy> circMonitor1{1};
    
    MonitorScreen<DisplayPolicy> monitor0{0};
    MonitorScreen<DisplayPolicy> monitor1{1};
    DualMonitorScreen<DisplayPolicy> dualMonitor;

    DisconnectedMsgScreen<DisplayPolicy> disconnectedMsg;

public:
    NativeViewPolicy(AppState& state) {}

    template <typename HWPolicy>
    void setupScreens(View<DisplayPolicy, HWPolicy>& appView) {
        appView.addConnectedScreen(&circMonitor0);
        appView.addConnectedScreen(&circMonitor1);
        
        appView.addConnectedScreen(&monitor0);
        appView.addConnectedScreen(&monitor1);
        appView.addConnectedScreen(&dualMonitor);
        
        appView.addDisconnectedScreen(&disconnectedMsg);
    }
};
