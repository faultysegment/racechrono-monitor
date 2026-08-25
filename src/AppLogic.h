#pragma once

#include "AppState.h"
#include "EventBus.h"
#include <string>
#include <cstring>
#include <cstdlib>

#define CMD_TYPE_REMOVE_ALL 0
#define CMD_TYPE_REMOVE 1
#define CMD_TYPE_ADD_INCOMPLETE 2
#define CMD_TYPE_ADD 3
#define CMD_TYPE_UPDATE_ALL 4
#define CMD_TYPE_UPDATE 5

#define CMD_RESULT_OK 0
#define CMD_RESULT_PAYLOAD_OUT_OF_SEQUENCE 1
#define CMD_RESULT_EQUATION_EXCEPTION 2

#define MAX_REMAINING_PAYLOAD 2048
#define MAX_PAYLOAD_PART 17

template <typename BLEPolicy, typename HWPolicy, typename StoragePolicy>
class AppLogic {
    AppState& state;
    EventBus& bus;
    BLEPolicy ble;

    bool wasConnected;

public:
    AppLogic(AppState& s, EventBus& b) : 
        state(s), bus(b), wasConnected(true) {
    }
    
    void setup() {
        HWPolicy::initBoard();

        StoragePolicy::init();
        state.speedLimit = StoragePolicy::getFloat("limit_speed", 5.0f);
        state.timeLimit = StoragePolicy::getFloat("limit_time", 0.1f);
        restoreDisconnectedScreen();
        restoreConnectedScreen();

        HWPolicy::initBattery();
        state.batteryPercent = HWPolicy::getBatteryPercent();

        bluetoothStart();
    }

    void pollInput() {
        static uint32_t buttonPressStartTime = 0;
        static bool buttonPressed = false;
        if (HWPolicy::isPowerKeyPressed()) {
            if (!buttonPressed) {
                buttonPressed = true;
                buttonPressStartTime = HWPolicy::millis();
                bus.push(Event{EventType::HW_PWR_BTN_PRESS, 0, 0, 0});
            } else if (HWPolicy::millis() - buttonPressStartTime >= 3000) {
                bus.push(Event{EventType::HW_PWR_BTN_LONG_PRESS, 0, 0, 0});
                buttonPressed = false; // Prevent repeated triggers
            }
        } else {
            if (buttonPressed) {
                bus.push(Event{EventType::HW_PWR_BTN_RELEASE, 0, 0, 0});
            }
            buttonPressed = false;
        }

        static bool lastActionBtn = false;
        static uint32_t actionBtnPressTime = 0;
        bool actionBtn = HWPolicy::isActionKeyPressed();
        
        if (!lastActionBtn && actionBtn) { // Pressed
            actionBtnPressTime = HWPolicy::millis();
        } else if (lastActionBtn && !actionBtn) { // Released
            uint32_t duration = HWPolicy::millis() - actionBtnPressTime;
            if (duration > 50 && duration < 1000) {
                bus.push(Event{EventType::HW_ACTION_TOGGLE, 0, 0, 0});
            }
        }
        lastActionBtn = actionBtn;

        int navDelta = HWPolicy::getNavigationDelta();
        if (navDelta != 0) {
            bus.push(Event{EventType::HW_NAV_DELTA, navDelta, 0, 0});
        }
        
        HWPolicy::pollExtraEvents(bus);
    }

    void pollLogic() {
        bool isConnected = (ble.getConnectedCount() > 0);
        if (wasConnected != isConnected) {
            wasConnected = isConnected;
            if (isConnected) {
                bus.push(Event{EventType::BLE_CONNECTED, 0, 0, 0});
            } else {
                bus.push(Event{EventType::BLE_DISCONNECTED, 0, 0, 0});
            }
        }

        static uint32_t lastBatteryUpdate = 0;
        if (HWPolicy::millis() - lastBatteryUpdate > 5000) {
            bus.push(Event{EventType::SYS_BATTERY_UPDATE, HWPolicy::getBatteryPercent(), 0, 0});
            lastBatteryUpdate = HWPolicy::millis();
        }
        
        // Configuration state machine tick
        if (isConnected && !state.isConfiguring && !state.isConfigured) {
            if (ble.isConfigIndicating()) {
                bus.push(Event{EventType::BLE_CONFIG_MONITOR, 0, 0, 0});
            }
        }
    }

    void processEvent(const Event& e) {
        switch (e.type) {
            case EventType::HW_PWR_BTN_LONG_PRESS:
                bus.push(Event{EventType::UI_SHOW_POWER_OFF, 0, 0, 0});
                HWPolicy::delay(500); 
                HWPolicy::powerOffBoard();
                break;

            case EventType::HW_ACTION_TOGGLE:
                toggleEditMode();
                break;

            case EventType::HW_NAV_DELTA: {
                if (state.isEditMode) {
                    changeValue(e.arg1);
                } else {
                    changePage(e.arg1);
                }
                break;
            }

            case EventType::BLE_CONNECTED:
                state.isConnected = true;
                state.reset();
                restoreConnectedScreen();
                bus.push(Event{EventType::UI_SHOW_CONNECTED, 0, 0, 0});
                break;

            case EventType::BLE_DISCONNECTED:
                state.isConnected = false;
                restoreDisconnectedScreen();
                bus.push(Event{EventType::UI_SHOW_DISCONNECTED, 0, 0, 0});
                bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
                ble.startAdvertising();
                break;

            case EventType::BLE_MONITOR_UPDATE:
                if (!e.str_arg.empty()) {
                    handleMonitorData(e.str_arg);
                } else {
                    state.setMonitorValue(e.arg1, e.arg2);
                }
                bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
                break;

            case EventType::SYS_BATTERY_UPDATE:
                state.batteryPercent = e.arg1;
                bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
                break;

            case EventType::BLE_CONFIG_MONITOR:
                if (e.str_arg.empty()) {
                    // Start configuring
                    state.isConfiguring = true;
                    bus.push(Event{EventType::UI_SHOW_CONFIGURING, 0, 0, 0});
                    if (configureMonitors()) {
                        bus.push(Event{EventType::UI_SHOW_CONFIG_DONE, 0, 0, 0});
                        state.isConfigured = true;
                        bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
                    } else {
                        bus.push(Event{EventType::UI_SHOW_CONFIG_FAIL, 0, 0, 0});
                    }
                } else {
                    // Process response
                    handleConfigData(e.str_arg);
                }
                break;

            default:
                break;
        }
    }

private:
    void restoreConnectedScreen() {
        int baseScreen = StoragePolicy::getInt("last_screen", 0);
        int hudMode = StoragePolicy::getInt("hud_mode", 0);
        int numConnected = state.numConnectedScreens;
        if (numConnected >= 2) {
            int m = numConnected / 2;
            baseScreen = (baseScreen % m + m) % m;
            state.currentScreenIndex = (hudMode != 0) ? (baseScreen + m) : baseScreen;
        } else {
            state.currentScreenIndex = baseScreen;
        }
    }

    void restoreDisconnectedScreen() {
        int hudMode = StoragePolicy::getInt("hud_mode", 0);
        int numDisc = state.numDisconnectedScreens;
        if (hudMode != 0 && numDisc >= 2) {
            state.disconnectedScreenIndex = numDisc / 2;
        } else {
            state.disconnectedScreenIndex = 0;
        }
    }

    void toggleEditMode() {
        state.isEditMode = !state.isEditMode;
        if (!state.isEditMode) {
            StoragePolicy::putFloat("limit_speed", state.speedLimit);
            StoragePolicy::putFloat("limit_time", state.timeLimit);
        }
        bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
    }

    void changePage(int navDelta) {
        if (state.isConnected) {
            int numConnected = state.numConnectedScreens;
            if (numConnected > 0) {
                int newIndex = (state.currentScreenIndex + navDelta) % numConnected;
                if (newIndex < 0) {
                    newIndex += numConnected;
                }
                state.currentScreenIndex = newIndex;
                if (numConnected >= 2) {
                    int m = numConnected / 2;
                    int baseScreen = state.currentScreenIndex % m;
                    bool isHud = (state.currentScreenIndex >= m);
                    StoragePolicy::putInt("last_screen", baseScreen);
                    StoragePolicy::putInt("hud_mode", isHud ? 1 : 0);
                } else {
                    StoragePolicy::putInt("last_screen", state.currentScreenIndex);
                }
            }
        } else {
            int numDisconnected = state.numDisconnectedScreens;
            if (numDisconnected > 0) {
                int newIndex = (state.disconnectedScreenIndex + navDelta) % numDisconnected;
                if (newIndex < 0) {
                    newIndex += numDisconnected;
                }
                state.disconnectedScreenIndex = newIndex;
                if (numDisconnected >= 2) {
                    int mDisc = numDisconnected / 2;
                    bool isHud = (state.disconnectedScreenIndex >= mDisc);
                    StoragePolicy::putInt("hud_mode", isHud ? 1 : 0);
                }
            }
        }
        bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
    }

    void changeValue(int navDelta) {
        float change = (navDelta > 0) ? 0.1f : -0.1f;
        if (state.isConnected) {
            int numConnected = state.numConnectedScreens;
            int baseIdx = state.currentScreenIndex;
            if (numConnected >= 2) {
                baseIdx = baseIdx % (numConnected / 2);
            }
            if (baseIdx < state.nextMonitorId) {
                float* limitPtr = state.monitors[baseIdx].limitPtr;
                if (limitPtr) {
                    *limitPtr += change;
                    if (*limitPtr < 0.1f) *limitPtr = 0.1f;
                }
            }
        } else {
            int numDisc = state.numDisconnectedScreens;
            int baseDisc = state.disconnectedScreenIndex;
            if (numDisc >= 2) {
                baseDisc = baseDisc % (numDisc / 2);
            }
            if (baseDisc == 1) { // Speed Config
                state.speedLimit += change;
                if (state.speedLimit < 0.1f) state.speedLimit = 0.1f;
            } else if (baseDisc == 2) { // Time Config
                state.timeLimit += change;
                if (state.timeLimit < 0.1f) state.timeLimit = 0.1f;
            }
        }
        bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
    }

    void handleConfigData(const std::string& rxData) {
        const uint8_t* data = (const uint8_t*)rxData.data();
        if (rxData.length() >= 1) {
            int result = data[0];
            int monitorId = rxData.length() >= 2 ? data[1] : 0;
            switch (result) {
                case CMD_RESULT_EQUATION_EXCEPTION:
                    state.setMonitorException(monitorId, true);
                    bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
                    break;
            }
        }
    }

    void handleMonitorData(const std::string& rxData) {
        const uint8_t* data = (const uint8_t*)rxData.data();
        int len = rxData.length();
        int dataPos = 0;
        while (dataPos + 5 <= len) {
            int monitorId = (int)data[dataPos];
            uint32_t raw_val = ((uint32_t)data[dataPos + 1] << 24) | 
                               ((uint32_t)data[dataPos + 2] << 16) | 
                               ((uint32_t)data[dataPos + 3] << 8)  | 
                                (uint32_t)data[dataPos + 4];
            int32_t value;
            memcpy(&value, &raw_val, sizeof(value));
            state.setMonitorValue(monitorId, value);
            dataPos += 5;
        }
    }

    void bluetoothStart() {
        uint8_t mac[6];
        HWPolicy::getMacDefault(mac);
        char name[255];
        snprintf(name, sizeof(name), "RC DIY #%02X%02X", mac[4], mac[5]);
        
        ble.init("RaceChrono Monitor", &bus);
        ble.startAdvertising();
    }

    bool sendConfigCommand(int cmdType, int monitorId, const char* payload, int payloadSequence = 0) {
        cmdType = cmdType == CMD_TYPE_ADD_INCOMPLETE ? CMD_TYPE_ADD : cmdType;

        char* remainingPayload = NULL; 
        char payloadPart[MAX_PAYLOAD_PART + 1];
        if (payload && cmdType == CMD_TYPE_ADD) {
            strncpy(payloadPart, payload, MAX_PAYLOAD_PART);
            payloadPart[MAX_PAYLOAD_PART] = '\0';
            
            int payloadLen = strlen(payload);
            if (payloadLen > MAX_PAYLOAD_PART) {
                int remainingPayloadLen = payloadLen - MAX_PAYLOAD_PART;
                remainingPayload = (char*)malloc(remainingPayloadLen + 1);
                strncpy(remainingPayload, payload + MAX_PAYLOAD_PART, remainingPayloadLen);
                remainingPayload[remainingPayloadLen] = '\0';
                cmdType = CMD_TYPE_ADD_INCOMPLETE;
            }
        } else {
            payloadPart[0] = '\0';
        }
      
        uint8_t bytes[20];
        bytes[0] = (uint8_t)cmdType;
        bytes[1] = (uint8_t)monitorId;
        bytes[2] = (uint8_t)payloadSequence;
        memcpy(bytes + 3, payloadPart, strlen(payloadPart));
        
        ble.indicateConfig(bytes, 3 + strlen(payloadPart));

        if (remainingPayload) {
            bool r = sendConfigCommand(CMD_TYPE_ADD, monitorId, remainingPayload, payloadSequence + 1);
            free(remainingPayload);
            return r;
        } else {
            return true;
        }
    }

    bool addMonitorConfig(const char* monitorName, const char* filterDef, float multiplier, const char* title, bool positiveIsGood, int decimals, float* limitPtr) {
        if (state.nextMonitorId < MAX_MONITORS) {
            if (!sendConfigCommand(CMD_TYPE_ADD, state.nextMonitorId, filterDef)) {
                return false;
            }
            return state.addMonitor(monitorName, multiplier, title, positiveIsGood, decimals, limitPtr);
        }
        return true;
    }

    bool configureMonitors() {
        state.resetMonitors();
        if (!addMonitorConfig("Delta curr lap time", "channel(device(lap), delta_lap_time)*100.0", 0.01, "TIME", false, 2, &state.timeLimit) ||
            !addMonitorConfig("Delta speed", "channel(device(calc), delta_speed)*100", 0.036, "SPEED", true, 1, &state.speedLimit)) {
            return false;
        }
        return true;    
    }
};
