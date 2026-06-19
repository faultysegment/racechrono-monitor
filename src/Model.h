#pragma once

#include <cstdint>


class Model {
public:
    static const int MAX_MONITORS = 255;
    static const int MONITOR_NAME_MAX = 32;
    static const int32_t INVALID_VALUE = 0x7fffffff;

    struct Monitor {
        char name[MONITOR_NAME_MAX + 1];
        char title[16];
        float multiplier;
        int32_t value;
        bool positiveIsGood;
        int decimals;
        float* limitPtr;
    };

    Monitor monitors[MAX_MONITORS];
    int nextMonitorId;
    
    bool isConnected;
    bool isConfiguring;
    bool isConfigured;
    int currentScreenIndex; // used when connected
    int disconnectedScreenIndex; // used when disconnected

    bool isEditMode;
    float timeLimit;
    float speedLimit;
    
    int batteryPercent;

    Model();
    void reset();
    bool addMonitor(const char* name, float multiplier, const char* title, bool positiveIsGood, int decimals, float* limitPtr);
    void setMonitorValue(int id, int32_t value);
    void resetMonitors();
};
