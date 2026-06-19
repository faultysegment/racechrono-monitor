#pragma once

#include <cstdint>


class Model {
public:
    static const int MAX_MONITORS = 255;
    static const int MONITOR_NAME_MAX = 32;
    static const int32_t INVALID_VALUE = 0x7fffffff;

    struct Monitor {
        char name[MONITOR_NAME_MAX + 1];
        float multiplier;
        int32_t value;
    };

    Monitor monitors[MAX_MONITORS];
    int nextMonitorId;
    
    bool isConnected;
    bool isConfiguring;
    bool isConfigured;
    
    int currentScreenIndex; // used when connected
    int disconnectedScreenIndex; // used when disconnected
    static const int NUM_SCREENS = 2;
    static const int NUM_DISCONNECTED_SCREENS = 3;

    bool isEditMode;
    float timeLimit;
    float speedLimit;
    
    int batteryPercent;

    Model();
    void reset();
    bool addMonitor(const char* name, float multiplier);
    void setMonitorValue(int id, int32_t value);
    void resetMonitors();
};
