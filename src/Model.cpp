#include "Model.h"
#include <cstring>


Model::Model() {
    isConnected = false;
    disconnectedScreenIndex = 0;
    isEditMode = false;
    batteryPercent = -1;
    reset();
}

void Model::reset() {
    isConfiguring = false;
    isConfigured = false;
    currentScreenIndex = 0;
    resetMonitors();
}

void Model::resetMonitors() {
    nextMonitorId = 0;
    for (int i = 0; i < MAX_MONITORS; i++) {
        monitors[i].value = INVALID_VALUE;
        monitors[i].multiplier = 0;
        memset(monitors[i].name, 0, sizeof(monitors[i].name));
        memset(monitors[i].title, 0, sizeof(monitors[i].title));
        monitors[i].positiveIsGood = false;
        monitors[i].decimals = 1;
        monitors[i].limitPtr = nullptr;
    }
}

bool Model::addMonitor(const char* name, float multiplier, const char* title, bool positiveIsGood, int decimals, float* limitPtr) {
    if (nextMonitorId < MAX_MONITORS) {
        strncpy(monitors[nextMonitorId].name, name, MONITOR_NAME_MAX);
        monitors[nextMonitorId].name[MONITOR_NAME_MAX] = '\0';
        strncpy(monitors[nextMonitorId].title, title, 15);
        monitors[nextMonitorId].title[15] = '\0';
        monitors[nextMonitorId].multiplier = multiplier;
        monitors[nextMonitorId].positiveIsGood = positiveIsGood;
        monitors[nextMonitorId].decimals = decimals;
        monitors[nextMonitorId].limitPtr = limitPtr;
        monitors[nextMonitorId].value = INVALID_VALUE;
        nextMonitorId++;
        return true;
    }
    return false;
}

void Model::setMonitorValue(int id, int32_t value) {
    if (id >= 0 && id < nextMonitorId) {
        monitors[id].value = value;
    }
}
