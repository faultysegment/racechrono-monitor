#pragma once
#include "../../AppState.h"
#include "../../View.h"
#include "../../Screens/CircularMonitorScreen.h"
#include "../../Screens/CircularDisconnectedScreen.h"

template <typename DisplayPolicy>
class AmoledViewPolicy {
    CircularMonitorScreen<DisplayPolicy> singleScreens[MAX_SCREENS];
    CircularDisconnectedScreen<DisplayPolicy> disconnectedMsg;

public:
    AmoledViewPolicy(AppState& state) {}

    template <typename HWPolicy>
    void setupScreens(View<DisplayPolicy, HWPolicy>& appView, AppState& state) {
        appView.clearScreens();
        for (int i = 0; i < state.numScreenConfigs && i < MAX_SCREENS; ++i) {
            const auto& sc = state.screenConfigs[i];
            singleScreens[i].setMonitorIndex(sc.primaryMonitorIndex);
            appView.addConnectedScreen(&singleScreens[i]);
        }
        appView.addDisconnectedScreen(&disconnectedMsg);
    }
};
