#pragma once
#include "../../AppState.h"
#include "../../View.h"
#include "../../Screens/CircularMonitorScreen.h"
#include "../../Screens/CircularDisconnectedScreen.h"

template <typename DisplayPolicy>
class AmoledViewPolicy {
    CircularMonitorScreen<DisplayPolicy> monitor0{0};
    CircularMonitorScreen<DisplayPolicy> monitor1{1};

    CircularDisconnectedScreen<DisplayPolicy> disconnectedMsg;

public:
    AmoledViewPolicy(AppState& state) {}

    template <typename HWPolicy>
    void setupScreens(View<DisplayPolicy, HWPolicy>& appView) {
        appView.addConnectedScreen(&monitor0);
        appView.addConnectedScreen(&monitor1);
        
        appView.addDisconnectedScreen(&disconnectedMsg);
    }
};
