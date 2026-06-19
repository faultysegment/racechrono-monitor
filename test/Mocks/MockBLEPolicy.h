#pragma once
#include <string>
#include <vector>
#include "../../src/Policies/BLEPolicyCallback.h"

class MockBLEPolicy {
    static BLEPolicyCallback* ctrl;
public:
    static int connectedCount;
    static bool indicating;
    static std::vector<std::string> sentConfigCommands;

    void init(const char* name, BLEPolicyCallback* controller) {
        ctrl = controller;
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
        ctrl = nullptr;
    }

    // Helpers to simulate BLE events in tests
    static void simulateConnect() {
        connectedCount = 1;
    }
    
    static void simulateDisconnect() {
        connectedCount = 0;
        if(ctrl) ctrl->onBLEDisconnected();
    }

    static void simulateConfigWrite(const std::string& data) {
        if(ctrl) ctrl->onConfigWrite(data);
    }

    static void simulateNotificationWrite(const std::string& data) {
        if(ctrl) ctrl->onNotificationWrite(data);
    }
};

#ifdef PIO_UNIT_TESTING
BLEPolicyCallback* MockBLEPolicy::ctrl = nullptr;
int MockBLEPolicy::connectedCount = 0;
bool MockBLEPolicy::indicating = false;
std::vector<std::string> MockBLEPolicy::sentConfigCommands;
#endif
