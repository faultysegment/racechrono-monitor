#pragma once

#include "Model.h"
#include <cmath>
#include "Screens/IScreen.h"
#include "Screens/TimeScreen.h"
#include "Screens/SpeedScreen.h"
#include "Screens/DisconnectedMsgScreen.h"
#include "Screens/ConfigTimeScreen.h"
#include "Screens/ConfigSpeedScreen.h"

template <typename DisplayPolicy, typename HWPolicy>
class View {
public:
    View(Model& m) : model(m), displayStarted(false), lastScreenIndex(-1), lastEditMode(false), lastConnected(false) {
        connectedScreens[0] = &timeScreen;
        connectedScreens[1] = &speedScreen;
        
        disconnectedScreens[0] = &disconnectedMsgScreen;
        disconnectedScreens[1] = &configSpeedScreen;
        disconnectedScreens[2] = &configTimeScreen;
    }
    
    void init() {
        tft.init();
        tft.setRotation(3);

#ifdef TFT_BL
        HWPolicy::pinMode(TFT_BL, 1); // 1 = OUTPUT
        setBacklight(true);
#endif

        tft.fillScreen(0x001F); // TFT_BLUE
        tft.setCursor(0, 0);
        tft.setTextWrap(false);
        tft.setTextSize(2);
    }
    
    void update() {
        int currentIdx = model.isConnected ? model.currentScreenIndex : model.disconnectedScreenIndex;
        IScreen<DisplayPolicy>* activeScreen = model.isConnected ? connectedScreens[currentIdx] : disconnectedScreens[currentIdx];

        if (!displayStarted || lastScreenIndex != currentIdx || lastEditMode != model.isEditMode || lastConnected != model.isConnected) {
            displayStarted = true;
            lastScreenIndex = currentIdx;
            lastEditMode = model.isEditMode;
            lastConnected = model.isConnected;
            
            activeScreen->onShow(tft, model);
            drawBattery(true);
        }
        
        activeScreen->onUpdate(tft, model);
        drawBattery(false);
    }

    void drawBattery(bool force) {
        static int lastBat = -2;
        if (force || lastBat != model.batteryPercent) {
            lastBat = model.batteryPercent;
            int screenW = tft.width();
            tft.setTextSize(2);
            tft.setTextColor(0xFFFF, 0x0000); 
            tft.setCursor(screenW - 55, 10); 
            char buf[16];
            if (model.batteryPercent >= 0 && model.batteryPercent <= 100) {
                snprintf(buf, sizeof(buf), "%3d%%", model.batteryPercent);
            } else {
                snprintf(buf, sizeof(buf), "---%%");
            }
            tft.print(buf);
        }
    }
    
    void showMessage(const char* msg, uint32_t color = 0xFFFF, uint32_t bg = 0x0000) {
        tft.fillScreen(bg);
        tft.setCursor(0, 0);
        tft.setTextColor(color, bg);
        tft.println(msg);
    }

    void showConnected() {
        displayStarted = false; 
        showMessage("BLE connected!", 0x001F, 0x0000); 
    }
    void showDisconnected() {
        showMessage("No BLE connection!", 0xFFFF, 0xF800); 
    }
    void showConfiguring() {
        tft.setTextColor(0xD69A, 0x0000); 
        tft.print("Configuring... ");
    }
    void showConfigDone() {
        tft.setTextColor(0xD69A, 0x0000);
        tft.println("Done");
    }
    void showConfigFail() {
        tft.setTextColor(0xF800, 0x0000); 
        tft.println("Fail");
    }
    void showPowerOff() {
        tft.fillScreen(0x0000); 
        tft.setTextColor(0xFFFF, 0x0000); 
        tft.setCursor(tft.width() / 2 - 60, tft.height() / 2 - 10);
        tft.print("Powering off...");
    }
    void showException(int monitorId, const char* msg) {
        tft.setTextColor(0xF800, 0x0000); 
        tft.println("");
        tft.print(monitorId);
        tft.print(" ");
        tft.println(msg);
    }
    
    void setBacklight(bool on) {
#ifdef TFT_BL
        HWPolicy::digitalWrite(TFT_BL, on ? 1 : 0); 
#endif
    }

private:
    Model& model;
    DisplayPolicy tft;
    bool displayStarted;
    int lastScreenIndex;
    bool lastEditMode;
    bool lastConnected;

    TimeScreen<DisplayPolicy> timeScreen;
    SpeedScreen<DisplayPolicy> speedScreen;
    IScreen<DisplayPolicy>* connectedScreens[Model::NUM_SCREENS];

    DisconnectedMsgScreen<DisplayPolicy> disconnectedMsgScreen;
    ConfigSpeedScreen<DisplayPolicy> configSpeedScreen;
    ConfigTimeScreen<DisplayPolicy> configTimeScreen;
    IScreen<DisplayPolicy>* disconnectedScreens[Model::NUM_DISCONNECTED_SCREENS];
};
