#pragma once

#include "AppState.h"
#include "EventBus.h"
#include "ColorUtils.h"
#include <ArduinoJson.h>
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
    bool wasIndicating;

public:
    AppLogic(AppState& s, EventBus& b) : 
        state(s), bus(b), wasConnected(true), wasIndicating(false) {
    }
    
    void setup() {
        StoragePolicy::init();
        loadConfig();
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
        
        bool isIndicating = isConnected && ble.isConfigIndicating();
        if (!wasIndicating && isIndicating) {
            // Re-subscription to indications (e.g. session restart or continuation)
            state.isConfigured = false;
        }
        wasIndicating = isIndicating;

        // Configuration state machine tick
        if (isConnected && !state.isConfigured && !state.isConfiguring(HWPolicy::millis())) {
            if (isIndicating) {
                bus.push(Event{EventType::BLE_CONFIG_MONITOR, 0, 0, 0});
            }
        }

        // WebUI heartbeat timeout check
        if (state.lastHeartbeatMillis != 0 && (HWPolicy::millis() - state.lastHeartbeatMillis >= AppState::CONFIG_MODE_TIMEOUT_MS)) {
            state.lastHeartbeatMillis = 0;
            bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
        }
    }

    void processEvent(const Event& e) {
        switch (e.type) {
            case EventType::HW_NAV_DELTA:
                changePage(e.arg1);
                break;

            case EventType::HW_ACTION_TOGGLE:
                // No-op
                break;

            case EventType::HW_PWR_BTN_LONG_PRESS:
                HWPolicy::powerOffBoard();
                break;

            case EventType::BLE_CONNECTED:
                state.isConnected = true;
                state.isConfigured = false;
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

            case EventType::EVENT_CONFIG_MODE_ENTER:
                state.lastHeartbeatMillis = HWPolicy::millis();
                bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
                break;

            case EventType::EVENT_CONFIG_MODE_EXIT:
                state.lastHeartbeatMillis = 0;
                bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
                break;

            case EventType::EVENT_CONFIG_RELOAD:
                state.lastHeartbeatMillis = 0;
                loadConfig();
                bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
                break;

            case EventType::EVENT_DEVICE_REBOOT:
                HWPolicy::reboot();
                break;

            default:
                break;
        }
    }

    void saveDefaultConfig() {
        const char* defaultJson = "{\n"
            "  \"isHud\": false,\n"
            "  \"webui\": {\n"
            "    \"enabled\": false,\n"
            "    \"ssid\": \"RaceChrono-Monitor\",\n"
            "    \"password\": \"\"\n"
            "  },\n"
            "  \"monitors\": [\n"
            "    {\n"
            "      \"id\": \"lap_delta\",\n"
            "      \"title\": \"TIME\",\n"
            "      \"formula\": \"channel(device(lap), delta_lap_time)*100.0\",\n"
            "      \"multiplier\": 0.01,\n"
            "      \"decimals\": 2,\n"
            "      \"limit\": 0.3\n"
            "    },\n"
            "    {\n"
            "      \"id\": \"speed_delta\",\n"
            "      \"title\": \"SPEED\",\n"
            "      \"formula\": \"channel(device(calc), delta_speed)*100\",\n"
            "      \"multiplier\": 0.036,\n"
            "      \"decimals\": 1,\n"
            "      \"limit\": 1.0\n"
            "    }\n"
            "  ],\n"
            "  \"screens\": [\n"
            "    {\n"
            "      \"type\": \"single\",\n"
            "      \"monitor\": \"lap_delta\",\n"
            "      \"positive_color\": \"#FF0000\",\n"
            "      \"negative_color\": \"#00FF00\",\n"
            "      \"title_color\": \"#0000FF\",\n"
            "      \"value_color\": \"#0000FF\"\n"
            "    },\n"
            "    {\n"
            "      \"type\": \"single\",\n"
            "      \"monitor\": \"speed_delta\",\n"
            "      \"positive_color\": \"#00FFFF\",\n"
            "      \"negative_color\": \"#FFA500\",\n"
            "      \"title_color\": \"#0000FF\",\n"
            "      \"value_color\": \"#0000FF\"\n"
            "    },\n"
            "    {\n"
            "      \"type\": \"dual\",\n"
            "      \"top\": {\n"
            "        \"monitor\": \"lap_delta\",\n"
            "        \"positive_color\": \"#FF0000\",\n"
            "        \"negative_color\": \"#00FF00\"\n"
            "      },\n"
            "      \"bottom\": {\n"
            "        \"monitor\": \"speed_delta\",\n"
            "        \"positive_color\": \"#00FFFF\",\n"
            "        \"negative_color\": \"#FFA500\"\n"
            "      }\n"
            "    }\n"
            "  ]\n"
            "}\n";
        StoragePolicy::writeConfigFile("/config.json", defaultJson);
    }

    void loadDefaultConfig() {
        state.isHud = false;
        state.clearMonitorConfigs();
        state.clearScreenConfigs();
        state.webuiConfig = WebUIConfig();

        MonitorConfig timeCfg;
        strncpy(timeCfg.id, "lap_delta", sizeof(timeCfg.id));
        timeCfg.id[sizeof(timeCfg.id) - 1] = '\0';
        strncpy(timeCfg.title, "TIME", sizeof(timeCfg.title));
        timeCfg.title[sizeof(timeCfg.title) - 1] = '\0';
        strncpy(timeCfg.formula, "channel(device(lap), delta_lap_time)*100.0", sizeof(timeCfg.formula));
        timeCfg.formula[sizeof(timeCfg.formula) - 1] = '\0';
        timeCfg.multiplier = 0.01f;
        timeCfg.positiveIsGood = false;
        timeCfg.decimals = 2;
        timeCfg.limit = 0.3f;
        state.addMonitorConfig(timeCfg);

        MonitorConfig speedCfg;
        strncpy(speedCfg.id, "speed_delta", sizeof(speedCfg.id));
        speedCfg.id[sizeof(speedCfg.id) - 1] = '\0';
        strncpy(speedCfg.title, "SPEED", sizeof(speedCfg.title));
        speedCfg.title[sizeof(speedCfg.title) - 1] = '\0';
        strncpy(speedCfg.formula, "channel(device(calc), delta_speed)*100", sizeof(speedCfg.formula));
        speedCfg.formula[sizeof(speedCfg.formula) - 1] = '\0';
        speedCfg.multiplier = 0.036f;
        speedCfg.positiveIsGood = true;
        speedCfg.decimals = 1;
        speedCfg.limit = 1.0f;
        state.addMonitorConfig(speedCfg);

        state.timeLimit = 0.3f;
        state.speedLimit = 1.0f;

        // Default 3 screens: single lap_delta, single speed_delta, dual
        state.addScreenConfig(ScreenConfig{ScreenType::SINGLE, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{}});
        state.addScreenConfig(ScreenConfig{ScreenType::SINGLE, ScreenSlotConfig{1, 0x07FF, 0xFD20, 0x001F, 0x001F}, ScreenSlotConfig{}});
        state.addScreenConfig(ScreenConfig{ScreenType::DUAL, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{1, 0x07FF, 0xFD20, 0x001F, 0x001F}});
    }

    void loadConfig() {
        std::string jsonStr = StoragePolicy::readConfigFile("/config.json");
        if (jsonStr.empty()) {
            if (StoragePolicy::isCardPresent()) {
                saveDefaultConfig();
            }
            loadDefaultConfig();
            return;
        }

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, jsonStr);
        if (err || !doc.is<JsonObject>()) {
            loadDefaultConfig();
            return;
        }

        state.isHud = doc["isHud"] | false;
        state.clearMonitorConfigs();
        state.clearScreenConfigs();

        if (doc["webui"].is<JsonObject>()) {
            JsonObject w = doc["webui"];
            state.webuiConfig.enabled = w["enabled"] | false;
            const char* ssid = w["ssid"] | "";
            const char* pass = w["password"] | "";
            strncpy(state.webuiConfig.ssid, ssid, sizeof(state.webuiConfig.ssid) - 1);
            state.webuiConfig.ssid[sizeof(state.webuiConfig.ssid) - 1] = '\0';
            strncpy(state.webuiConfig.password, pass, sizeof(state.webuiConfig.password) - 1);
            state.webuiConfig.password[sizeof(state.webuiConfig.password) - 1] = '\0';
        } else {
            state.webuiConfig = WebUIConfig();
        }

        JsonArray monitors = doc["monitors"];
        if (monitors.isNull() || monitors.size() == 0) {
            loadDefaultConfig();
            return;
        }

        for (JsonObject m : monitors) {
            MonitorConfig cfg;
            const char* id = m["id"] | "";
            const char* title = m["title"] | "";
            const char* formula = m["formula"] | "";
            strncpy(cfg.id, id, sizeof(cfg.id));
            cfg.id[sizeof(cfg.id) - 1] = '\0';
            strncpy(cfg.title, title, sizeof(cfg.title));
            cfg.title[sizeof(cfg.title) - 1] = '\0';
            strncpy(cfg.formula, formula, sizeof(cfg.formula));
            cfg.formula[sizeof(cfg.formula) - 1] = '\0';
            cfg.multiplier = m["multiplier"] | 1.0f;
            cfg.positiveIsGood = m["positive_is_good"] | false;
            cfg.decimals = m["decimals"] | 1;
            cfg.limit = m["limit"] | 0.1f;
            state.addMonitorConfig(cfg);
        }

        if (state.numMonitorConfigs == 0) {
            loadDefaultConfig();
            return;
        }

        if (state.numMonitorConfigs >= 1) state.timeLimit = state.monitorConfigs[0].limit;
        if (state.numMonitorConfigs >= 2) state.speedLimit = state.monitorConfigs[1].limit;

        // Parse screens array
        JsonArray screens = doc["screens"];
        if (!screens.isNull()) {
            for (JsonObject s : screens) {
                const char* typeStr = s["type"] | "single";
                if (strcmp(typeStr, "dual") == 0) {
                    ScreenSlotConfig topSlot;
                    ScreenSlotConfig btmSlot;

                    if (s["top"].is<JsonObject>()) {
                        JsonObject topObj = s["top"];
                        topSlot.monitorIndex = state.findMonitorIndexById(topObj["monitor"] | "");
                        topSlot.positiveColor = ColorUtils::parseHexColor565(topObj["positive_color"] | "", 0xF800);
                        topSlot.negativeColor = ColorUtils::parseHexColor565(topObj["negative_color"] | "", 0x07E0);
                        topSlot.titleColor = ColorUtils::parseHexColor565(topObj["title_color"] | "", 0x001F);
                        topSlot.valueColor = ColorUtils::parseHexColor565(topObj["value_color"] | "", 0x001F);
                    } else {
                        topSlot.monitorIndex = state.findMonitorIndexById(s["top"] | "");
                        topSlot.positiveColor = ColorUtils::parseHexColor565(s["top_positive_color"] | "", 0xF800);
                        topSlot.negativeColor = ColorUtils::parseHexColor565(s["top_negative_color"] | "", 0x07E0);
                    }

                    if (s["bottom"].is<JsonObject>()) {
                        JsonObject btmObj = s["bottom"];
                        btmSlot.monitorIndex = state.findMonitorIndexById(btmObj["monitor"] | "");
                        btmSlot.positiveColor = ColorUtils::parseHexColor565(btmObj["positive_color"] | "", 0x07FF);
                        btmSlot.negativeColor = ColorUtils::parseHexColor565(btmObj["negative_color"] | "", 0xFD20);
                        btmSlot.titleColor = ColorUtils::parseHexColor565(btmObj["title_color"] | "", 0x001F);
                        btmSlot.valueColor = ColorUtils::parseHexColor565(btmObj["value_color"] | "", 0x001F);
                    } else {
                        btmSlot.monitorIndex = state.findMonitorIndexById(s["bottom"] | "");
                        btmSlot.positiveColor = ColorUtils::parseHexColor565(s["bottom_positive_color"] | "", 0x07FF);
                        btmSlot.negativeColor = ColorUtils::parseHexColor565(s["bottom_negative_color"] | "", 0xFD20);
                    }

                    if (topSlot.monitorIndex >= 0 && btmSlot.monitorIndex >= 0) {
                        state.addScreenConfig(ScreenConfig{ScreenType::DUAL, topSlot, btmSlot});
                    }
                } else { // "single"
                    const char* monId = s["monitor"] | "";
                    int monIdx = state.findMonitorIndexById(monId);
                    if (monIdx >= 0) {
                        uint16_t posColor = ColorUtils::parseHexColor565(s["positive_color"] | "", 0xF800);
                        uint16_t negColor = ColorUtils::parseHexColor565(s["negative_color"] | "", 0x07E0);
                        uint16_t titleColor = ColorUtils::parseHexColor565(s["title_color"] | "", 0x001F);
                        uint16_t valColor = ColorUtils::parseHexColor565(s["value_color"] | "", 0x001F);
                        state.addScreenConfig(ScreenConfig{ScreenType::SINGLE, ScreenSlotConfig{monIdx, posColor, negColor, titleColor, valColor}, ScreenSlotConfig{}});
                    }
                }
            }
        }

        // Fallback: If screens is omitted or empty
        if (state.numScreenConfigs == 0) {
            for (int i = 0; i < state.numMonitorConfigs; ++i) {
                state.addScreenConfig(ScreenConfig{ScreenType::SINGLE, ScreenSlotConfig{i, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{}});
            }
            if (state.numMonitorConfigs >= 2) {
                state.addScreenConfig(ScreenConfig{ScreenType::DUAL, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{1, 0x07FF, 0xFD20, 0x001F, 0x001F}});
            }
        }
    }

private:
    void restoreConnectedScreen() {
        int baseScreen = StoragePolicy::getInt("last_screen", 0);
        int numConnected = state.numConnectedScreens;
        if (numConnected > 0) {
            state.currentScreenIndex = (baseScreen % numConnected + numConnected) % numConnected;
        } else {
            state.currentScreenIndex = 0;
        }
    }

    void restoreDisconnectedScreen() {
        state.disconnectedScreenIndex = 0;
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
                StoragePolicy::putInt("last_screen", state.currentScreenIndex);
            }
        } else {
            int numDisconnected = state.numDisconnectedScreens;
            if (numDisconnected > 0) {
                int newIndex = (state.disconnectedScreenIndex + navDelta) % numDisconnected;
                if (newIndex < 0) {
                    newIndex += numDisconnected;
                }
                state.disconnectedScreenIndex = newIndex;
            }
        }
        bus.push(Event{EventType::UI_UPDATE, 0, 0, 0});
    }

    void handleConfigData(const std::string& rxData) {
        const uint8_t* data = (const uint8_t*)rxData.data();
        if (rxData.length() >= 1) {
            int cmdOrResult = data[0];
            int monitorId = rxData.length() >= 2 ? data[1] : 0;
            switch (cmdOrResult) {
                case CMD_TYPE_REMOVE_ALL:
                    // If 1 byte payload: CMD_TYPE_REMOVE_ALL command from RaceChrono
                    if (rxData.length() == 1) {
                        state.isConfigured = false;
                        bus.push(Event{EventType::BLE_CONFIG_MONITOR, 0, 0, 0});
                    }
                    break;
                case CMD_TYPE_UPDATE_ALL:
                case CMD_TYPE_UPDATE:
                    // Reconfigure request from RaceChrono (session start / continue)
                    state.isConfigured = false;
                    bus.push(Event{EventType::BLE_CONFIG_MONITOR, 0, 0, 0});
                    break;
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
        for (int i = 0; i < state.numMonitorConfigs; i++) {
            if (!addMonitorConfig(state.monitorConfigs[i].id,
                                  state.monitorConfigs[i].formula,
                                  state.monitorConfigs[i].multiplier,
                                  state.monitorConfigs[i].title,
                                  state.monitorConfigs[i].positiveIsGood,
                                  state.monitorConfigs[i].decimals,
                                  &state.monitorConfigs[i].limit)) {
                return false;
            }
        }
        return true;    
    }
};
