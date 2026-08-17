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
#include "../../App.h"
#include "Policies/AmoledViewPolicy.h"

Arduino_DataBus *bus = NULL;
Arduino_GFX *gfx = NULL;
Arduino_Canvas *canvas = NULL;

App<AmoledDisplayPolicy, AmoledHWPolicy, AmoledBLEPolicy, AmoledStoragePolicy, AmoledViewPolicy<AmoledDisplayPolicy>> app;

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

void setup() {
    app.setup();

    // Pin UI to Core 1 (App Core) to dedicate it for SPI/display rendering
    xTaskCreatePinnedToCore(uiTask, "UI_Task", 4096, NULL, 1, NULL, 1);
    // Pin Input and Logic to Core 0 (Pro Core) where BLE/WiFi usually runs
    xTaskCreatePinnedToCore(inputTask, "Input_Task", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(logicTask, "Logic_Task", 4096, NULL, 1, NULL, 0);
}

void loop() {
    // The main loop task can just be suspended, we do all work in our specific tasks
    vTaskSuspend(NULL);
}
