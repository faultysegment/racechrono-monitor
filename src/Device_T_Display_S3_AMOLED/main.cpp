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
        vTaskDelay(pdMS_TO_TICKS(5)); // High frequency 200Hz input polling
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

    // Pin Input to Core 0 (PRO_CPU) for dedicated high-priority I2C touch polling
    xTaskCreatePinnedToCore(inputTask, "Input_Task", 4096, NULL, 5, NULL, 0);

    // Pin UI Rendering to Core 1 (APP_CPU) for dedicated display rendering
    xTaskCreatePinnedToCore(uiTask, "UI_Task", 4096, NULL, 1, NULL, 1);
    
    // Pin Logic to Core 0 (PRO_CPU)
    xTaskCreatePinnedToCore(logicTask, "Logic_Task", 4096, NULL, 1, NULL, 0);
}

void loop() {
    // The main loop task can just be suspended, we do all work in our specific tasks
    vTaskSuspend(NULL);
}
