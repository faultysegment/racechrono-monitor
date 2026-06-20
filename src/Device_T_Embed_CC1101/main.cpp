#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <Preferences.h>

#include "App.h"

App app;

void uiTask(void* pvParameters) {
    Event e;
    while (1) {
        if (globalEventBus.pop_with_timeout(e, 25)) {
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
    xTaskCreatePinnedToCore(inputTask, "Input_Task", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(logicTask, "Logic_Task", 4096, NULL, 1, NULL, 0);
}

void loop() {
    // The main loop task can just be suspended, we do all work in our specific tasks
    vTaskSuspend(NULL);
}
