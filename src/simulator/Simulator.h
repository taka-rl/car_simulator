#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "../core/Config.h"
#include "../shaders/RectShader.h"
#include "../Loader.h"
#include "../entities/Entity.h"
#include "../renderers/Renderer.h"
#include "../vehicledynamics/BicycleModel.h"
#include "../vehicledynamics/VehicleTypes.h"
#include "../utilities/Randomizer.h"
#include "../envs/ParkingEnv.h"


// forward declarations at global scope
struct Observation;


/**
 * Parking Env Class
 * ---------------------------
 * This class compose the simulations in this project.  
 */
class Simulator {

public:
    // constrotuctor
    Simulator(GLFWwindow* window);

    // necessary simulation attributes and methods
    // Init simulator
    bool init();

    // Simulation run loop
    void run();
       
private:

    // Windows and timing
    GLFWwindow* window{nullptr};
    int fbW = 0, fbH = 0;

    // Core systems
    VehicleParams vehicleParams;
    Randomizer randomizer;
    ParkingEnv env;
    Action action;

    // Renderer
    std::unique_ptr<RectShader> rectShader;
    std::unique_ptr<Loader> quad;
    std::unique_ptr<Renderer> renderer;

    // Scene entities
    Entity carEntity;
    Entity parkingEntity;
    Entity wheelFL, wheelFR, wheelRL, wheelRR;
    std::vector<Entity> trajectoryEntities;
    std::array<std::array<float, 2>, 4> anchors;

    // Simulation
    const double simDt{0.01};
    double accumulator{0.0};
    double lastTime{0.0};
    Position2D prevState{};
    Position2D curState{};
    float prevPsi{0.0f};
    float curPsi{0.0f};
    float prevDelta{0.0f};
    float curDelta{0.0f};

    void initRenderer();         // Loader + RectShader + Renderer
    void initSimulationState();  // simDt, accumulator, VehicleParams, BicycleModel, VehicleState
    void initEntities();         // car / parking / wheels / trajectory

    void placeWheel(Entity& wheel, float ax, float ay, bool front,
                    const Position2D& pos, const float& yawDraw, const float& steer);

    /** Step simulation
     * one frame: input → env steps → update Entities → render.

    */ 
    void tick();

    void draw(const Position2D& posDraw, const float& yawDraw, const float& deltaDraw);
    

    // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
    void processInput(GLFWwindow *window, Action& action);

    // glfw: whenever the window size changed (by OS or user resize) this callback function executes
    // ---------------------------------------------------------------------------------------------
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    // Clamp accumulator to avoid spiral of death after stalls
    inline void clampAccumulator(double& accum, const double simDt, double maxSteps = 5.0) {
        const double MAX_ACCUM = simDt * maxSteps;
        if (accum > MAX_ACCUM) accum = MAX_ACCUM;
    }

    // Linear interpolation for positions
    inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

    // Interpolate Position2D (positions); for headings, use lerpAngle on psi
    inline Position2D interp(const Position2D& prev, const Position2D& curr, float alpha) {
        return Position2D{
            lerp(prev.x, curr.x, alpha),
            lerp(prev.y, curr.y, alpha)
        };
    }

    // keep the entire rectangle visible on screen.
    // ------------------------------------------------------------------------
    inline void keepOnScreenMeters(Position2D& s, float width_m, float length_m, int fbW, int fbH, float ppm) {
        const float worldHalfW = (fbW / ppm) * 0.5f;
        const float worldHalfH = (fbH / ppm) * 0.5f;
        const float marginX = width_m * 0.5f;
        const float marginY = length_m * 0.5f;
        s.x = std::clamp(s.x, -worldHalfW + marginX, worldHalfW - marginX);
        s.y = std::clamp(s.y, -worldHalfH + marginY, worldHalfH - marginY);
    };
    
};
#endif
