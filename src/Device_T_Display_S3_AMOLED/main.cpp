#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <BLEDevice.h>
#include <Preferences.h>
#include <Arduino_GFX_Library.h>

#include "Policies/AmoledDisplayPolicy.h"
#include "Policies/AmoledHWPolicy.h"
#include "Policies/AmoledBLEPolicy.h"
#include "Policies/AmoledStoragePolicy.h"
#include "Policies/AmoledWebConfigPolicy.h"
#include "../../App.h"
#include "Policies/AmoledViewPolicy.h"

App<AmoledDisplayPolicy, AmoledHWPolicy, AmoledBLEPolicy, AmoledStoragePolicy, AmoledViewPolicy<AmoledDisplayPolicy>> app;
AmoledWebConfigPolicy<AmoledStoragePolicy> webConfig;

void uiTask(void* pvParameters) {
    Event e;
    while (1) {
        if (app.getEventBus().pop_with_timeout(e, 25)) {
            app.processEvent(e);
        } else {
            app.tickUI();
        }
    }
}

void inputTask(void* pvParameters) {
    while (1) {
        app.pollInput();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void logicTask(void* pvParameters) {
    while (1) {
        app.pollLogic();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void webTask(void* pvParameters) {
    webConfig.begin(app.getState(), app.getEventBus(), app.getStorage());
    while (1) {
        webConfig.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setup() {
    app.setup();

    app.getEventBus().push(Event{EventType::UI_UPDATE, 0, 0, 0});

    xTaskCreatePinnedToCore(uiTask, "UI_Task", 8192, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(logicTask, "Logic_Task", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(inputTask, "Input_Task", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(webTask, "Web_Task", 4096, NULL, 1, NULL, 0);
}

void loop() {
    vTaskSuspend(NULL);
}
