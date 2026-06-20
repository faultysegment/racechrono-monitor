#include "App.h"

App app;

int main(int argc, char* argv[]) {
    app.setup();
    while (app.isRunning()) {
        app.loop();
    }
    return 0;
}
