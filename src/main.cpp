#ifndef PIO_UNIT_TESTING

#include "App.h"

App app;

#ifdef ARDUINO
#include <Arduino.h>

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
#endif // PIO_UNIT_TESTING
