#pragma once
#include "AppState.h"
#include "View.h"
#include "Screens/MonitorScreen.h"
#include "Screens/DualMonitorScreen.h"
#include "Screens/ConfigMonitorScreen.h"
#include "Screens/CircularMonitorScreen.h"
#include "Screens/CircularDualMonitorScreen.h"
#include "Screens/DisconnectedMsgScreen.h"
#include "Screens/CircularConfigMonitorScreen.h"
#include "Screens/HudScreenWrapper.h"

template <typename DisplayPolicy>
class NativeViewPolicy {
    CircularMonitorScreen<DisplayPolicy> circMonitor0{0};
    CircularMonitorScreen<DisplayPolicy> circMonitor1{1};
    CircularDualMonitorScreen<DisplayPolicy> circDualMonitor;
    
    MonitorScreen<DisplayPolicy> monitor0{0};
    MonitorScreen<DisplayPolicy> monitor1{1};
    DualMonitorScreen<DisplayPolicy> dualMonitor;

    HudScreenWrapper<DisplayPolicy> hudCircMonitor0{&circMonitor0};
    HudScreenWrapper<DisplayPolicy> hudCircMonitor1{&circMonitor1};
    HudScreenWrapper<DisplayPolicy> hudCircDualMonitor{&circDualMonitor};
    HudScreenWrapper<DisplayPolicy> hudMonitor0{&monitor0};
    HudScreenWrapper<DisplayPolicy> hudMonitor1{&monitor1};
    HudScreenWrapper<DisplayPolicy> hudDualMonitor{&dualMonitor};
    
    DisconnectedMsgScreen<DisplayPolicy> disconnectedMsg;
    
    CircularConfigMonitorScreen<DisplayPolicy> circConfigSpeed{1};
    CircularConfigMonitorScreen<DisplayPolicy> circConfigTime{2};
    
    ConfigMonitorScreen<DisplayPolicy> configSpeed;
    ConfigMonitorScreen<DisplayPolicy> configTime;

    HudScreenWrapper<DisplayPolicy> hudDisconnectedMsg{&disconnectedMsg};
    HudScreenWrapper<DisplayPolicy> hudCircConfigSpeed{&circConfigSpeed};
    HudScreenWrapper<DisplayPolicy> hudCircConfigTime{&circConfigTime};
    HudScreenWrapper<DisplayPolicy> hudConfigSpeed{&configSpeed};
    HudScreenWrapper<DisplayPolicy> hudConfigTime{&configTime};

public:
    NativeViewPolicy(AppState& state) 
        : configSpeed("SPEED LIMIT", &state.speedLimit),
          configTime("TIME LIMIT", &state.timeLimit) {}

    template <typename HWPolicy>
    void setupScreens(View<DisplayPolicy, HWPolicy>& appView) {
        appView.addConnectedScreen(&circMonitor0);
        appView.addConnectedScreen(&circMonitor1);
        appView.addConnectedScreen(&circDualMonitor);
        
        appView.addConnectedScreen(&monitor0);
        appView.addConnectedScreen(&monitor1);
        appView.addConnectedScreen(&dualMonitor);

        appView.addConnectedScreen(&hudCircMonitor0);
        appView.addConnectedScreen(&hudCircMonitor1);
        appView.addConnectedScreen(&hudCircDualMonitor);
        appView.addConnectedScreen(&hudMonitor0);
        appView.addConnectedScreen(&hudMonitor1);
        appView.addConnectedScreen(&hudDualMonitor);
        
        appView.addDisconnectedScreen(&disconnectedMsg);
        
        appView.addDisconnectedScreen(&circConfigSpeed);
        appView.addDisconnectedScreen(&circConfigTime);
        
        appView.addDisconnectedScreen(&configSpeed);
        appView.addDisconnectedScreen(&configTime);

        appView.addDisconnectedScreen(&hudDisconnectedMsg);
        appView.addDisconnectedScreen(&hudCircConfigSpeed);
        appView.addDisconnectedScreen(&hudCircConfigTime);
        appView.addDisconnectedScreen(&hudConfigSpeed);
        appView.addDisconnectedScreen(&hudConfigTime);
    }
};
