#pragma once
#include <string>
#include <vector>
#include "../../EventBus.h"

class MockBLEPolicy {
    static EventBus* bus;
public:
    static int connectedCount;
    static bool indicating;
    static std::vector<std::string> sentConfigCommands;

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

    static void reset() {
        connectedCount = 0;
        indicating = false;
        sentConfigCommands.clear();
        bus = nullptr;
    }

    // Helpers to simulate BLE events in tests
    static void simulateConnect() {
        connectedCount = 1;
    }
    
    static void simulateDisconnect() {
        connectedCount = 0;
        if(bus) bus->push(Event{EventType::BLE_DISCONNECTED, 0, 0, 0});
    }

    static void simulateConfigWrite(const std::string& data) {
        if(bus) bus->push(Event{EventType::BLE_CONFIG_MONITOR, 0, 0, 0, data});
    }

    static void simulateNotificationWrite(const std::string& data) {
        if(bus) bus->push(Event{EventType::BLE_MONITOR_UPDATE, 0, 0, 0, data});
    }
};

#ifdef PIO_UNIT_TESTING
EventBus* MockBLEPolicy::bus = nullptr;
int MockBLEPolicy::connectedCount = 0;
bool MockBLEPolicy::indicating = false;
std::vector<std::string> MockBLEPolicy::sentConfigCommands;
#endif
