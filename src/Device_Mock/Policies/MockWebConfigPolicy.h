#pragma once
#include "../../AppState.h"
#include "../../EventBus.h"

template <typename StoragePolicy>
class MockWebConfigPolicy {
public:
    bool isRunning = false;
    void reset() { isRunning = false; }
    void begin(AppState& state, EventBus& bus, StoragePolicy& storage) {
        if (state.webuiConfig.enabled && state.webuiConfig.ssid[0] != '\0') {
            isRunning = true;
        }
    }
    void handleClient() {}
    void stop() { isRunning = false; }
};
