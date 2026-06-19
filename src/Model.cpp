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
    }
}

bool Model::addMonitor(const char* name, float multiplier) {
    if (nextMonitorId < MAX_MONITORS) {
        strncpy(monitors[nextMonitorId].name, name, MONITOR_NAME_MAX);
        monitors[nextMonitorId].name[MONITOR_NAME_MAX] = '\0';
        monitors[nextMonitorId].multiplier = multiplier;
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
