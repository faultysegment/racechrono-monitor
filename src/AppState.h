#pragma once

#include <cstdint>
#include <cstring>

#define MAX_MONITORS 8
#define MAX_SCREENS 8
#define MONITOR_ID_MAX_LEN 16
#define MONITOR_NAME_MAX 31

struct MonitorConfig {
    char id[MONITOR_ID_MAX_LEN + 1];
    char title[16];
    char formula[128];
    float multiplier;
    bool positiveIsGood;
    int decimals;
    float limit;
};

enum class ScreenType {
    SINGLE,
    DUAL
};

struct ScreenSlotConfig {
    int monitorIndex;
    uint16_t positiveColor;
    uint16_t negativeColor;
    uint16_t titleColor;
    uint16_t valueColor;

    ScreenSlotConfig(int idx = -1, uint16_t posColor = 0xF800, uint16_t negColor = 0x07E0, uint16_t tColor = 0x001F, uint16_t vColor = 0x001F)
        : monitorIndex(idx), positiveColor(posColor), negativeColor(negColor), titleColor(tColor), valueColor(vColor) {}
};

struct ScreenConfig {
    ScreenType type;
    ScreenSlotConfig primary;
    ScreenSlotConfig secondary;
};

struct WebUIConfig {
    bool enabled;
    char ssid[32];
    char password[64];

    WebUIConfig() : enabled(false) {
        ssid[0] = '\0';
        password[0] = '\0';
    }
};

class AppState {
public:
    static const int32_t INVALID_VALUE = 2147483647;

    struct Monitor {
        char name[32];
        char title[16];
        float multiplier;
        int32_t value;
        bool positiveIsGood;
        int decimals;
        float limit;
        float* limitPtr;
        bool hasException;
    };

    Monitor monitors[MAX_MONITORS];
    int nextMonitorId;

    MonitorConfig monitorConfigs[MAX_MONITORS];
    int numMonitorConfigs;

    ScreenConfig screenConfigs[MAX_SCREENS];
    int numScreenConfigs;

    WebUIConfig webuiConfig;

    bool isHud;

    bool isConnected;
    bool isConfiguring;
    bool isConfigured;
    
    int currentScreenIndex;
    int disconnectedScreenIndex;
    
    int batteryPercent;

    int numConnectedScreens;
    int numDisconnectedScreens;

    float speedLimit;
    float timeLimit;

    AppState() {
        isConnected = false;
        disconnectedScreenIndex = 0;
        isHud = false;
        numMonitorConfigs = 0;
        numScreenConfigs = 0;
        batteryPercent = -1;
        numConnectedScreens = 0;
        numDisconnectedScreens = 0;
        speedLimit = 5.0f;
        timeLimit = 0.1f;
        reset();
    }

    void reset() {
        isConfiguring = false;
        isConfigured = false;
        currentScreenIndex = 0;
        webuiConfig = WebUIConfig();
        resetMonitors();
    }

    void clearMonitorConfigs() {
        numMonitorConfigs = 0;
    }

    bool addMonitorConfig(const MonitorConfig& cfg) {
        if (numMonitorConfigs < MAX_MONITORS) {
            monitorConfigs[numMonitorConfigs++] = cfg;
            return true;
        }
        return false;
    }

    void clearScreenConfigs() {
        numScreenConfigs = 0;
    }

    bool addScreenConfig(const ScreenConfig& cfg) {
        if (numScreenConfigs < MAX_SCREENS) {
            screenConfigs[numScreenConfigs++] = cfg;
            return true;
        }
        return false;
    }

    int findMonitorIndexById(const char* id) const {
        if (!id) return -1;
        for (int i = 0; i < numMonitorConfigs; ++i) {
            if (strncmp(monitorConfigs[i].id, id, MONITOR_ID_MAX_LEN) == 0) {
                return i;
            }
        }
        return -1;
    }

    void resetMonitors() {
        nextMonitorId = 0;
        for (int i = 0; i < MAX_MONITORS; i++) {
            monitors[i].value = INVALID_VALUE;
            monitors[i].multiplier = 0;
            memset(monitors[i].name, 0, sizeof(monitors[i].name));
            memset(monitors[i].title, 0, sizeof(monitors[i].title));
            monitors[i].positiveIsGood = false;
            monitors[i].decimals = 1;
            monitors[i].limit = 0.1f;
            monitors[i].limitPtr = &monitors[i].limit;
            monitors[i].hasException = false;
        }
    }

    bool addMonitor(const char* name, float multiplier, const char* title, bool positiveIsGood, int decimals, float* limitPtr) {
        if (nextMonitorId < MAX_MONITORS) {
            strncpy(monitors[nextMonitorId].name, name, MONITOR_NAME_MAX);
            monitors[nextMonitorId].name[MONITOR_NAME_MAX] = '\0';
            strncpy(monitors[nextMonitorId].title, title, 15);
            monitors[nextMonitorId].title[15] = '\0';
            monitors[nextMonitorId].multiplier = multiplier;
            monitors[nextMonitorId].positiveIsGood = positiveIsGood;
            monitors[nextMonitorId].decimals = decimals;
            if (limitPtr) {
                monitors[nextMonitorId].limit = *limitPtr;
                monitors[nextMonitorId].limitPtr = limitPtr;
            } else {
                monitors[nextMonitorId].limitPtr = &monitors[nextMonitorId].limit;
            }
            monitors[nextMonitorId].value = INVALID_VALUE;
            monitors[nextMonitorId].hasException = false;
            nextMonitorId++;
            return true;
        }
        return false;
    }

    void setMonitorValue(int id, int32_t value) {
        if (id >= 0 && id < nextMonitorId) {
            monitors[id].value = value;
            monitors[id].hasException = false;
        }
    }

    void setMonitorException(int id, bool exception) {
        if (id >= 0 && id < nextMonitorId) {
            monitors[id].hasException = exception;
        }
    }
};
