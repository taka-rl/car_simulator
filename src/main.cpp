#include "core/Config.h"
#include "Window.h"
#include "simulator/Simulator.h"


int main() {
    // Create window + OpenGL context
    Window window(SCR_WIDTH, SCR_HEIGHT, "Car Simulator");
    if (!window.isValid()) return -1;

    // Create simulator after OpenGL is ready
    Simulator sim(window.get());
    
    // Init simulator
    if (!sim.init()) return -1;
    sim.run();
    
    return 0;
}
