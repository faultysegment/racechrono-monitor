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
    CircularMonitorScreen<DisplayPolicy> circMonitor2{2};
    CircularMonitorScreen<DisplayPolicy> circMonitor3{3};
    
    MonitorScreen<DisplayPolicy> monitor0{0};
    MonitorScreen<DisplayPolicy> monitor1{1};
    MonitorScreen<DisplayPolicy> monitor2{2};
    MonitorScreen<DisplayPolicy> monitor3{3};
    DualMonitorScreen<DisplayPolicy> dualMonitor;

    DisconnectedMsgScreen<DisplayPolicy> disconnectedMsg;

public:
    NativeViewPolicy(AppState& state) {}

    template <typename HWPolicy>
    void setupScreens(View<DisplayPolicy, HWPolicy>& appView, AppState& state) {
        appView.clearScreens();
        int count = (state.numMonitorConfigs > 0) ? state.numMonitorConfigs : 2;
        if (count == 1) {
            appView.addConnectedScreen(&circMonitor0);
            appView.addConnectedScreen(&monitor0);
        } else {
            if (count >= 1) appView.addConnectedScreen(&circMonitor0);
            if (count >= 2) appView.addConnectedScreen(&circMonitor1);
            if (count >= 3) appView.addConnectedScreen(&circMonitor2);
            if (count >= 4) appView.addConnectedScreen(&circMonitor3);
            
            if (count >= 1) appView.addConnectedScreen(&monitor0);
            if (count >= 2) appView.addConnectedScreen(&monitor1);
            if (count >= 3) appView.addConnectedScreen(&monitor2);
            if (count >= 4) appView.addConnectedScreen(&monitor3);
            if (count >= 2) appView.addConnectedScreen(&dualMonitor);
        }
        
        appView.addDisconnectedScreen(&disconnectedMsg);
    }
};
