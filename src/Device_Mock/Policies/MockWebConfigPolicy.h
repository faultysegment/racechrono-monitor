#pragma once
#include "../../AppState.h"
#include "../../EventBus.h"

template <typename StoragePolicy>
class MockWebConfigPolicy {
public:
    static bool isRunning;
    static void reset() { isRunning = false; }
    void begin(AppState& state, EventBus& bus) {
        if (state.webuiConfig.enabled && state.webuiConfig.ssid[0] != '\0') {
            isRunning = true;
        }
    }
    void handleClient() {}
    void stop() { isRunning = false; }
};

#ifdef PIO_UNIT_TESTING
template <typename StoragePolicy>
bool MockWebConfigPolicy<StoragePolicy>::isRunning = false;
#endif
