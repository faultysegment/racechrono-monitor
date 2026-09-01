#pragma once
#include "AppState.h"
#include "View.h"
#include "Screens/MonitorScreen.h"
#include "Screens/DualMonitorScreen.h"
#include "Screens/CircularMonitorScreen.h"
#include "Screens/DisconnectedMsgScreen.h"
#include "Screens/ConfiguringScreen.h"

template <typename DisplayPolicy>
class NativeViewPolicy {
    MonitorScreen<DisplayPolicy> singleScreens[MAX_SCREENS];
    CircularMonitorScreen<DisplayPolicy> circScreens[MAX_SCREENS];
    DualMonitorScreen<DisplayPolicy> dualScreens[MAX_SCREENS];
    DisconnectedMsgScreen<DisplayPolicy> disconnectedMsg;
    ConfiguringScreen<DisplayPolicy> configuringScreen;

public:
    NativeViewPolicy(AppState& state) {}

    template <typename HWPolicy>
    void setupScreens(View<DisplayPolicy, HWPolicy>& appView, AppState& state) {
        appView.clearScreens();
        for (int i = 0; i < state.numScreenConfigs && i < MAX_SCREENS; ++i) {
            const auto& sc = state.screenConfigs[i];
            if (sc.type == ScreenType::SINGLE) {
                circScreens[i].setConfig(sc.primary);
                appView.addConnectedScreen(&circScreens[i]);
                singleScreens[i].setConfig(sc.primary);
                appView.addConnectedScreen(&singleScreens[i]);
            } else if (sc.type == ScreenType::DUAL) {
                dualScreens[i].setSlots(sc.primary, sc.secondary);
                appView.addConnectedScreen(&dualScreens[i]);
            }
        }
        appView.addDisconnectedScreen(&disconnectedMsg);
        appView.setConfiguringScreen(&configuringScreen);
    }
};
