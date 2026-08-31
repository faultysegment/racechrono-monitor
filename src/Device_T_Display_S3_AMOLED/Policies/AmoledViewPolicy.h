#pragma once
#include "../../AppState.h"
#include "../../View.h"
#include "../../Screens/CircularMonitorScreen.h"
#include "../../Screens/CircularDisconnectedScreen.h"

template <typename DisplayPolicy>
class AmoledViewPolicy {
    CircularMonitorScreen<DisplayPolicy> monitor0{0};
    CircularMonitorScreen<DisplayPolicy> monitor1{1};
    CircularMonitorScreen<DisplayPolicy> monitor2{2};
    CircularMonitorScreen<DisplayPolicy> monitor3{3};

    CircularDisconnectedScreen<DisplayPolicy> disconnectedMsg;

public:
    AmoledViewPolicy(AppState& state) {}

    template <typename HWPolicy>
    void setupScreens(View<DisplayPolicy, HWPolicy>& appView, AppState& state) {
        appView.clearScreens();
        int count = (state.numMonitorConfigs > 0) ? state.numMonitorConfigs : 2;
        if (count >= 1) appView.addConnectedScreen(&monitor0);
        if (count >= 2) appView.addConnectedScreen(&monitor1);
        if (count >= 3) appView.addConnectedScreen(&monitor2);
        if (count >= 4) appView.addConnectedScreen(&monitor3);
        
        appView.addDisconnectedScreen(&disconnectedMsg);
    }
};
