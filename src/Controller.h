#pragma once

#include "Model.h"
#include "BLEPolicyCallback.h"
#include <string>
#include <cstring>
#include <cstdlib>

// ... skipped defines for brevity ...

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

template <typename ModelType, typename ViewType, typename BLEPolicy, typename HWPolicy, typename StoragePolicy>
class Controller : public BLEPolicyCallback {
public:
    Controller(Model& m, ViewType& v) : 
        model(m), view(v), buttonPressStartTime(0), buttonPressed(false), 
        displayUpdateCount(0), wasConnected(true) {
    }
    
    void setup() {
        HWPolicy::initBoard();

        StoragePolicy::init();
        model.speedLimit = StoragePolicy::getFloat("limit_speed", 5.0f);
        model.timeLimit = StoragePolicy::getFloat("limit_time", 0.1f);

        HWPolicy::initBattery();
        model.batteryPercent = HWPolicy::getBatteryPercent();

        view.init();
        bluetoothStart();
    }

    void loop() {
        if (HWPolicy::isPowerKeyPressed()) {
            if (!buttonPressed) {
                buttonPressed = true;
                buttonPressStartTime = HWPolicy::millis();
            } else if (HWPolicy::millis() - buttonPressStartTime >= 3000) {
                view.showPowerOff();
                HWPolicy::delay(500); 

                view.setBacklight(false);
                HWPolicy::powerOffBoard();
            }
        } else {
            buttonPressed = false;
        }

        bool isConnected = (ble.getConnectedCount() > 0);
        if (wasConnected != isConnected) {
            wasConnected = isConnected;

            if (isConnected) {
                model.isConnected = true;
                onBLEConnected();
            } else {
                model.isConnected = false;
                view.showDisconnected();
            }
        }

        static bool lastActionBtn = false;
        static uint32_t actionBtnPressTime = 0;
        bool actionBtn = HWPolicy::isActionKeyPressed();
        
        if (!lastActionBtn && actionBtn) { // Pressed
            actionBtnPressTime = HWPolicy::millis();
        } else if (lastActionBtn && !actionBtn) { // Released
            uint32_t duration = HWPolicy::millis() - actionBtnPressTime;
            if (duration > 50 && duration < 1000) { 
                model.isEditMode = !model.isEditMode;
                if (!model.isEditMode) {
                    StoragePolicy::putFloat("limit_speed", model.speedLimit);
                    StoragePolicy::putFloat("limit_time", model.timeLimit);
                }
                displayUpdateCount = 8; // force update
            }
        }
        lastActionBtn = actionBtn;

        int navDelta = HWPolicy::getNavigationDelta();
        if (navDelta != 0) {
            if (model.isEditMode) {
                float change = (navDelta > 0) ? 0.1f : -0.1f;
                if (isConnected) {
                    if (model.currentScreenIndex == 0) {
                        model.timeLimit += change;
                        if (model.timeLimit < 0.1f) model.timeLimit = 0.1f;
                    } else if (model.currentScreenIndex == 1) {
                        model.speedLimit += change;
                        if (model.speedLimit < 0.1f) model.speedLimit = 0.1f;
                    }
                } else {
                    if (model.disconnectedScreenIndex == 1) { // Speed Config
                        model.speedLimit += change;
                        if (model.speedLimit < 0.1f) model.speedLimit = 0.1f;
                    } else if (model.disconnectedScreenIndex == 2) { // Time Config
                        model.timeLimit += change;
                        if (model.timeLimit < 0.1f) model.timeLimit = 0.1f;
                    }
                }
            } else {
                if (isConnected) {
                    if (navDelta > 0) {
                        model.currentScreenIndex = (model.currentScreenIndex + 1) % Model::NUM_SCREENS;
                    } else {
                        model.currentScreenIndex = (model.currentScreenIndex - 1 + Model::NUM_SCREENS) % Model::NUM_SCREENS;
                    }
                } else {
                    if (navDelta > 0) {
                        model.disconnectedScreenIndex = (model.disconnectedScreenIndex + 1) % Model::NUM_DISCONNECTED_SCREENS;
                    } else {
                        model.disconnectedScreenIndex = (model.disconnectedScreenIndex - 1 + Model::NUM_DISCONNECTED_SCREENS) % Model::NUM_DISCONNECTED_SCREENS;
                    }
                }
            }
            displayUpdateCount = 8; // Force immediate display update
        }

        if (isConnected) {
            if (handleConfigure()) {
                displayUpdateCount++;
                if (displayUpdateCount >= 8) {
                    view.update();
                    displayUpdateCount = 0;   
                }
            }
        } else {
            displayUpdateCount++;
            if (displayUpdateCount >= 8) {
                view.update();
                displayUpdateCount = 0;   
            }
        }

        static uint32_t lastBatteryUpdate = 0;
        if (HWPolicy::millis() - lastBatteryUpdate > 5000) {
            model.batteryPercent = HWPolicy::getBatteryPercent();
            lastBatteryUpdate = HWPolicy::millis();
        }

        HWPolicy::delay(25);
    }

    // BLE Callbacks
    void onBLEConnected() {
        model.reset();
        displayUpdateCount = 0;
        view.showConnected();
    }

    void onBLEDisconnected() {
        ble.startAdvertising();
    }

    void onConfigWrite(std::string rxData) {
        const uint8_t* data = (const uint8_t*)rxData.data();
        if (rxData.length() >= 1) {
            int result = data[0];
            int monitorId = rxData.length() >= 2 ? data[1] : 0;
            switch (result) {
                case CMD_RESULT_PAYLOAD_OUT_OF_SEQUENCE:
                    view.showException(monitorId, "out-of-sequence");
                    break;
                case CMD_RESULT_EQUATION_EXCEPTION:
                    view.showException(monitorId, "exception");
                    break;
                default:
                    break;
            }
        }
    }

    void onNotificationWrite(std::string rxData) {
        const uint8_t* data = (const uint8_t*)rxData.data();
        int len = rxData.length();
        int dataPos = 0;
        while (dataPos + 5 <= len) {
            int monitorId = (int)data[dataPos];
            int32_t value = data[dataPos + 1] << 24 | data[dataPos + 2] << 16 | data[dataPos + 3] << 8 | data[dataPos + 4];
            model.setMonitorValue(monitorId, value);
            dataPos += 5;
        }
    }

private:
    Model& model;
    ViewType& view;
    BLEPolicy ble;
    
    uint32_t buttonPressStartTime;
    bool buttonPressed;
    int displayUpdateCount;
    bool wasConnected;

    void bluetoothStart() {
        uint8_t mac[6];
        HWPolicy::getMacDefault(mac);
        char name[255];
        snprintf(name, sizeof(name), "RC DIY #%02X%02X", mac[4], mac[5]);
        
        ble.init(name, this);
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

    bool addMonitorConfig(const char* monitorName, const char* filterDef, float multiplier) {
        if (model.nextMonitorId < Model::MAX_MONITORS) {
            if (!sendConfigCommand(CMD_TYPE_ADD, model.nextMonitorId, filterDef)) {
                return false;
            }
            return model.addMonitor(monitorName, multiplier);
        }
        return true;
    }

    bool configureMonitors() {
        model.resetMonitors();
        if (!addMonitorConfig("Delta curr lap time", "channel(device(lap), delta_lap_time)*100.0", 0.01) ||
            !addMonitorConfig("Delta speed", "channel(device(calc), delta_speed)*100", 0.036)) {
            return false;
        }
        return true;    
    }

    bool handleConfigure() {
        if (!model.isConfiguring && !model.isConfigured) {
            if (ble.isConfigIndicating()) {
                model.isConfiguring = true;
                while (1) {
                    view.showConfiguring();
                    if ((ble.getConnectedCount() == 0) || configureMonitors()) {
                        view.showConfigDone();
                        model.isConfigured = true;
                        return true;
                    }
                    view.showConfigFail();
                }
            }
            return false;
        }
        return model.isConfigured;
    }
};
