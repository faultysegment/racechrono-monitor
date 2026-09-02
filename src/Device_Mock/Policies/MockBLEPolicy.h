#pragma once
#include <string>
#include <vector>
#include "../../EventBus.h"

class MockBLEPolicy {
    EventBus* bus = nullptr;
public:
    int connectedCount = 0;
    bool indicating = false;
    std::vector<std::string> sentConfigCommands;

    void init(const char* name, EventBus* b) {
        bus = b;
    }

    void startAdvertising() {}

    int getConnectedCount() {
        return connectedCount;
    }

    bool isConfigIndicating() {
        return indicating;
    }

    void indicateConfig(uint8_t* data, size_t len) {
        sentConfigCommands.push_back(std::string((char*)data, len));
    }

    void reset() {
        connectedCount = 0;
        indicating = false;
        sentConfigCommands.clear();
        bus = nullptr;
    }

    // Helpers to simulate BLE events in tests
    void simulateConnect() {
        connectedCount = 1;
    }
    
    void simulateDisconnect() {
        connectedCount = 0;
        if(bus) bus->push(Event{EventType::BLE_DISCONNECTED, 0, 0, 0});
    }

    void simulateConfigWrite(const std::string& data) {
        if(bus) bus->push(Event{EventType::BLE_CONFIG_MONITOR, 0, 0, 0, data});
    }

    void simulateNotificationWrite(const std::string& data) {
        if(bus) bus->push(Event{EventType::BLE_MONITOR_UPDATE, 0, 0, 0, data});
    }
};
