#pragma once

#include "AppState.h"
#include <cmath>
#include <vector>
#include "IScreen.h"
#include "EventBus.h"

template <typename DisplayPolicy, typename HWPolicy>
class View {
public:
    View(AppState& s) : state(s), displayStarted(false), lastScreenIndex(-1), lastEditMode(false), lastConnected(false) {}

    void addConnectedScreen(IScreen<DisplayPolicy>* screen) {
        connectedScreens.push_back(screen);
        state.numConnectedScreens = connectedScreens.size();
    }

    void addDisconnectedScreen(IScreen<DisplayPolicy>* screen) {
        disconnectedScreens.push_back(screen);
        state.numDisconnectedScreens = disconnectedScreens.size();
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
                if (!state.isConnected || state.isConfigured) {
                    update();
                }
                break;
            case EventType::UI_SHOW_CONNECTED:
                displayStarted = false; 
                showMessage("BLE connected!", 0x001F, 0x0000); 
                break;
            case EventType::UI_SHOW_DISCONNECTED:
                showMessage("Disconnected", 0xFFFF, 0xF800); 
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
        int currentIdx = state.isConnected ? state.currentScreenIndex : state.disconnectedScreenIndex;
        IScreen<DisplayPolicy>* activeScreen = state.isConnected ? connectedScreens[currentIdx] : disconnectedScreens[currentIdx];

        if (!displayStarted || lastScreenIndex != currentIdx || lastEditMode != state.isEditMode || lastConnected != state.isConnected) {
            displayStarted = true;
            lastScreenIndex = currentIdx;
            lastEditMode = state.isEditMode;
            lastConnected = state.isConnected;
            
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
    bool lastEditMode;
    bool lastConnected;

    std::vector<IScreen<DisplayPolicy>*> connectedScreens;
    std::vector<IScreen<DisplayPolicy>*> disconnectedScreens;
};

