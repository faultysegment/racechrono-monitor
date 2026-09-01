#pragma once
#include "../../AppState.h"
#include "../../EventBus.h"

template <typename StoragePolicy>
class RealWebConfigPolicy {
public:
    void begin(AppState& state, EventBus& bus) {}
    void handleClient() {}
    void stop() {}
};
