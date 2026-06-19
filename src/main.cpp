#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <Preferences.h>
#endif

#include "App.h"

App app;

#ifdef ARDUINO
void setup() {
    app.setup();
}

void loop() {
    app.loop();
}

#else

int main(int argc, char* argv[]) {
    app.setup();
    while (app.isRunning()) {
        app.loop();
    }
    return 0;
}

#endif
