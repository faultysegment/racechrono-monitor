#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <Preferences.h>
#include <lvgl.h>
#include <esp_timer.h>

#include "Policies/RealDisplayPolicy.h"
#include "Policies/RealHWPolicy.h"
#include "Policies/RealBLEPolicy.h"
#include "Policies/RealStoragePolicy.h"
#include "Policies/RealWebConfigPolicy.h"
#include "../App.h"
#include "Policies/TEmbedViewPolicy.h"

App<RealDisplayPolicy, RealHWPolicy, RealBLEPolicy, RealStoragePolicy, TEmbedViewPolicy<RealDisplayPolicy>> app;
RealWebConfigPolicy<RealStoragePolicy> webConfig;

static void lv_tick_task(void* arg) {
    lv_tick_inc(1);
}

void uiTask(void* pvParameters) {
    Event e;
    while (1) {
        if (app.getEventBus().pop_with_timeout(e, 5)) {
            app.processEvent(e);
        } else {
            app.tickUI();
        }
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
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

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000));

    // Pin UI to Core 1 (App Core) to dedicate it for SPI/display rendering
    xTaskCreatePinnedToCore(uiTask, "UI_Task", 8192, NULL, 1, NULL, 1);
    // Pin Input, Logic, and WebUI to Core 0 (Pro Core) where BLE/WiFi runs
    xTaskCreatePinnedToCore(inputTask, "Input_Task", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(logicTask, "Logic_Task", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(webTask, "Web_Task", 4096, NULL, 1, NULL, 0);
}

void loop() {
    // The main loop task can just be suspended, we do all work in our specific tasks
    vTaskSuspend(NULL);
}
