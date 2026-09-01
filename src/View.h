#pragma once

#include "AppState.h"
#include <cmath>
#include <vector>
#include "Screens/IScreen.h"
#include "EventBus.h"

template <typename DisplayPolicy, typename HWPolicy>
class View {
public:
    View(AppState& s) : state(s), displayStarted(false), lastScreenIndex(-1), lastConnected(false), lastConfiguring(false), configuringScreen(nullptr) {}

    void addConnectedScreen(IScreen<DisplayPolicy>* screen) {
        connectedScreens.push_back(screen);
        state.numConnectedScreens = connectedScreens.size();
    }

    void addDisconnectedScreen(IScreen<DisplayPolicy>* screen) {
        disconnectedScreens.push_back(screen);
        state.numDisconnectedScreens = disconnectedScreens.size();
    }

    void setConfiguringScreen(IScreen<DisplayPolicy>* screen) {
        configuringScreen = screen;
    }

    void clearScreens() {
        connectedScreens.clear();
        disconnectedScreens.clear();
        configuringScreen = nullptr;
        state.numConnectedScreens = 0;
        state.numDisconnectedScreens = 0;
        lastScreenIndex = -1;
    }

    int getNumConnectedScreens() const {
        return connectedScreens.size();
    }

    int getNumDisconnectedScreens() const {
        return disconnectedScreens.size();
    }
    
    void init() {
        tft.init();
        tft.setRotation(3);

        tft.setBacklight(true);

        tft.fillScreen(0x001F); // TFT_BLUE
        tft.setCursor(0, 0);
        tft.setTextWrap(false);
        tft.setTextSize(2);
    }
    
    void processEvent(const Event& e) {
        switch (e.type) {
            case EventType::UI_UPDATE:
                if (!state.isConnected || state.isConfigured || state.isConfiguring) {
                    update();
                }
                break;
            case EventType::UI_SHOW_CONNECTED:
                displayStarted = false; 
                showMessage("BLE connected!", 0x001F, 0x0000); 
                break;
            case EventType::UI_SHOW_DISCONNECTED:
                displayStarted = false; 
                showMessage("Disconnected", 0xF800, 0x0000); 
                break;
            case EventType::UI_SHOW_CONFIGURING:
                tft.setTextColor(0xD69A, 0x0000); 
                tft.print("Configuring... ");
                break;
            case EventType::UI_SHOW_CONFIG_DONE:
                tft.setTextColor(0xD69A, 0x0000);
                tft.println("Done");
                break;
            case EventType::UI_SHOW_CONFIG_FAIL:
                tft.setTextColor(0xF800, 0x0000); 
                tft.println("Fail");
                break;
            case EventType::HW_PWR_BTN_LONG_PRESS:
            case EventType::UI_SHOW_POWER_OFF:
                tft.fillScreen(0x0000); 
                tft.setTextColor(0xFFFF, 0x0000); 
                tft.setCursor(tft.width() / 2 - 60, tft.height() / 2 - 10);
                tft.print("Powering off...");
                tft.flush();
                break;

            default:
                break;
        }
    }
    
    void setBacklight(bool on) {
        tft.setBacklight(on);
    }

private:
    void update() {
        int currentIdx = 0;
        IScreen<DisplayPolicy>* activeScreen = nullptr;
        if (state.isConfiguring && configuringScreen) {
            activeScreen = configuringScreen;
            currentIdx = 999;
        } else if (state.isConnected) {
            if (!connectedScreens.empty()) {
                currentIdx = state.currentScreenIndex % connectedScreens.size();
                activeScreen = connectedScreens[currentIdx];
            }
        } else {
            if (!disconnectedScreens.empty()) {
                currentIdx = state.disconnectedScreenIndex % disconnectedScreens.size();
                activeScreen = disconnectedScreens[currentIdx];
            }
        }
        if (!activeScreen) return;

        tft.setHudMode(state.isHud);

        if (!displayStarted || lastScreenIndex != currentIdx || lastConnected != state.isConnected || lastConfiguring != state.isConfiguring) {
            displayStarted = true;
            lastScreenIndex = currentIdx;
            lastConnected = state.isConnected;
            lastConfiguring = state.isConfiguring;
            
            activeScreen->onShow(tft, state);
            tft.drawBattery(state.batteryPercent, true);
        }
        
        activeScreen->onUpdate(tft, state);
        tft.drawBattery(state.batteryPercent, false);
        tft.flush();
    }
    
    void showMessage(const char* msg, uint32_t color = 0xFFFF, uint32_t bg = 0x0000) {
        tft.fillScreen(bg);
        tft.setCursor(0, 0);
        tft.setTextColor(color, bg);
        tft.println(msg);
    }
    AppState& state;
    DisplayPolicy tft;
    bool displayStarted;
    int lastScreenIndex;
    bool lastConnected;
    bool lastConfiguring;

    IScreen<DisplayPolicy>* configuringScreen;
    std::vector<IScreen<DisplayPolicy>*> connectedScreens;
    std::vector<IScreen<DisplayPolicy>*> disconnectedScreens;
};
