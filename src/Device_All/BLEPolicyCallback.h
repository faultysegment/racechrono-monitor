#pragma once

#include <string>

class BLEPolicyCallback {
public:
    virtual void onBLEConnected() = 0;
    virtual void onBLEDisconnected() = 0;
    virtual void onConfigWrite(std::string rxData) = 0;
    virtual void onNotificationWrite(std::string rxData) = 0;
};
