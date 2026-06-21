#include "Policies/RealDisplayPolicy.h"
#include "Policies/RealHWPolicy.h"
#include "Policies/RealBLEPolicy.h"
#include "Policies/RealStoragePolicy.h"
#include "../App.h"
#include "Policies/NativeViewPolicy.h"

App<RealDisplayPolicy, RealHWPolicy, RealBLEPolicy, RealStoragePolicy, NativeViewPolicy<RealDisplayPolicy>> app;

int main(int argc, char* argv[]) {
    app.setup();
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        app.pollInput();
        app.pollLogic();

        Event e;
        while (app.getEventBus().try_pop(e)) {
            app.processEvent(e);
        }
        app.tickUI();
        
        SDL_Delay(25);
    }
    return 0;
}
