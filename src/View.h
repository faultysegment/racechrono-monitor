#pragma once

#include "Model.h"
#include <cmath>
#include <vector>
#include "IScreen.h"

template <typename DisplayPolicy, typename HWPolicy>
class View {
public:
    View(Model& m) : model(m), displayStarted(false), lastScreenIndex(-1), lastEditMode(false), lastConnected(false) {}

    void addConnectedScreen(IScreen<DisplayPolicy>* screen) {
        connectedScreens.push_back(screen);
    }

    void addDisconnectedScreen(IScreen<DisplayPolicy>* screen) {
        disconnectedScreens.push_back(screen);
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
            tft.drawBattery(model.batteryPercent, true);
        }
        
        activeScreen->onUpdate(tft, model);
        tft.drawBattery(model.batteryPercent, false);
        tft.flush();
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

    std::vector<IScreen<DisplayPolicy>*> connectedScreens;
    std::vector<IScreen<DisplayPolicy>*> disconnectedScreens;
};
