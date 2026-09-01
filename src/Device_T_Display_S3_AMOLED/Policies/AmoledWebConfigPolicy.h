#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "../../AppState.h"
#include "../../EventBus.h"
#include "../../WebUIContent.h"

template <typename StoragePolicy>
class AmoledWebConfigPolicy {
    WebServer* server = nullptr;
    EventBus* bus = nullptr;
    bool isRunning = false;

public:
    AmoledWebConfigPolicy() : server(nullptr), bus(nullptr), isRunning(false) {}

    ~AmoledWebConfigPolicy() {
        stop();
    }

    void begin(AppState& state, EventBus& eventBus) {
        if (!state.webuiConfig.enabled || state.webuiConfig.ssid[0] == '\0') {
            WiFi.mode(WIFI_OFF);
            return;
        }

        bus = &eventBus;

        IPAddress local_IP(192, 168, 1, 1);
        IPAddress gateway(192, 168, 1, 1);
        IPAddress subnet(255, 255, 255, 0);

        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(local_IP, gateway, subnet);
        const char* pass = (strlen(state.webuiConfig.password) >= 8) ? state.webuiConfig.password : nullptr;
        WiFi.softAP(state.webuiConfig.ssid, pass);

        server = new WebServer(80);

        server->on("/", HTTP_GET, [this]() {
            if (bus) bus->push(Event{EventType::EVENT_CONFIG_MODE_ENTER, 0, 0, 0});
            server->send_P(200, "text/html", WebUIContent::INDEX_HTML);
        });

        server->on("/saved", HTTP_GET, [this]() {
            if (bus) bus->push(Event{EventType::EVENT_CONFIG_MODE_EXIT, 0, 0, 0});
            server->send_P(200, "text/html", WebUIContent::SAVED_HTML);
        });

        server->on("/api/config", HTTP_GET, [this]() {
            std::string content = StoragePolicy::readConfigFile("/config.json");
            if (content.empty()) {
                content = "{}";
            }
            server->send(200, "application/json", content.c_str());
        });

        server->on("/api/config", HTTP_POST, [this]() {
            if (!server->hasArg("plain")) {
                server->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Empty body\"}");
                return;
            }
            String body = server->arg("plain");
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, body.c_str());
            if (err) {
                server->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
                return;
            }

            StoragePolicy::writeConfigFile("/config.json", body.c_str());
            if (bus) bus->push(Event{EventType::EVENT_CONFIG_RELOAD, 0, 0, 0});
            server->send(200, "application/json", "{\"status\":\"ok\"}");
        });

        server->on("/api/restart", HTTP_POST, [this]() {
            server->send(200, "application/json", "{\"status\":\"rebooting\"}");
            if (bus) bus->push(Event{EventType::EVENT_DEVICE_REBOOT, 0, 0, 0});
        });

        server->begin();
        isRunning = true;
    }

    void handleClient() {
        if (server && isRunning) {
            server->handleClient();
        }
    }

    void stop() {
        if (server) {
            server->stop();
            delete server;
            server = nullptr;
        }
        if (isRunning) {
            WiFi.mode(WIFI_OFF);
            isRunning = false;
        }
    }
};
