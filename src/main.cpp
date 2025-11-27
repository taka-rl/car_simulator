#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <algorithm>
#include <array>
#include <cmath>
#include <random>

#include "shaders/RectShader.h"
#include "Loader.h"
#include "entities/Entity.h"
#include "renderers/Renderer.h"
#include "vehicledynamics/BicycleModel.h"
#include "vehicledynamics/VehicleTypes.h"
#include "core/Config.h"
#include "utilities/MathUtils.h"
#include "utilities/Randomizer.h"
#include "envs/ParkingEnv.h"



// simulation state
// simulation state stays as meters

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Action& action);


// Clamp accumulator to avoid spiral of death after stalls
inline void clampAccumulator(double& accum, const double simDt, double maxSteps = 5.0) {
    const double MAX_ACCUM = simDt * maxSteps;
    if (accum > MAX_ACCUM) accum = MAX_ACCUM;
}

// Linear interpolation for positions
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

// Interpolate state (positions); for headings, use lerpAngle on psi
inline State interp(const State& prev, const State& curr, float alpha) {
    return State{
        lerp(prev.x, curr.x, alpha),
        lerp(prev.y, curr.y, alpha)
    };
}

// keep the entire rectangle visible on screen.
// ------------------------------------------------------------------------
inline void keepOnScreenMeters(State& s, float width_m, float length_m, int fbW, int fbH, float ppm) {
    const float worldHalfW = (fbW / ppm) * 0.5f;
    const float worldHalfH = (fbH / ppm) * 0.5f;
    const float marginX = width_m * 0.5f;
    const float marginY = length_m * 0.5f;
    s.x = std::clamp(s.x, -worldHalfW + marginX, worldHalfW - marginX);
    s.y = std::clamp(s.y, -worldHalfH + marginY, worldHalfH - marginY);
};

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // after gladLoadGLLoader(...)
    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);
    // (optional) also trigger your callback once to keep all logic in one place:
    framebuffer_size_callback(window, fbW, fbH);

    // build and compile our shader program
    // ------------------------------------
    RectShader rectShader;

    float vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f,  -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f, 0.5f, 0.0f   // top left 
        };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    // set up vertex data (and buffer(s)) and configure vertex attributes
    Loader quad(vertices, sizeof(vertices)/sizeof(float), indices, sizeof(indices)/sizeof(unsigned int));
    
    // grab uniform location once
    // carShader.use();
    // parkingShader.use();
    // int uOffsetLoc = glGetUniformLocation(carShader.getShaderID(), "uOffset");

    // Turn on vsync 60FPS
    glfwSwapInterval(1);

    // Simulation Config
    const double simDt = 0.01;
    double accumulator = 0.0;
    double lastTime = glfwGetTime();

    // wheels
    VehicleParams vehicleParams;
    vehicleParams.finalize();

    // vehicle state
    VehicleState vehicleState;

    // previous and current state for interpolation
    State prevState = vehicleState.pos, curState = vehicleState.pos;
    float prevPsi = vehicleState.psi, curPsi = vehicleState.psi;
    float prevDelta = vehicleState.delta, curDelta = vehicleState.delta;

    // kinematic model
    BicycleModel bikeModel(vehicleParams.Lf + vehicleParams.Lr);

    // inputs
    Action action;

    // wheel anchors in car-local frame
    const std::array<float, 2> anchors[4] = {
        {+vehicleParams.Lf, +vehicleParams.track*0.5f}, {+vehicleParams.Lf, -vehicleParams.track*0.5f},
        {-vehicleParams.Lr, -vehicleParams.track*0.5f}, {-vehicleParams.Lr, +vehicleParams.track*0.5f}
    };

    // renderer
    Renderer renderer(PPM, fbW, fbH);

    // random positions for car and parking
    Randomizer randomizer;
    ParkingEnv parkingEnv(&randomizer);
    const State randParkingPos = parkingEnv.setParkingPos(-15.f, 15.f, -10.f, 10.f);
    const float marginX = randomizer.randFloat(-5.0, 5.0), marginY = randomizer.randFloat(-5.0, 5.0);
    const State randCarPos = {randParkingPos.x + marginX, randParkingPos.y + marginY};   

    // entities
    Entity carEntity(&quad, &rectShader);
    carEntity.setColor({0.15f, 0.65f, 0.15f, 1.0f});
    carEntity.setYaw(vehicleState.psi);
    carEntity.setWidth(CAR_LENGTH);
    carEntity.setLength(CAR_WIDTH);
    carEntity.setPos(randCarPos);

    Entity parkingEntity(&quad, &rectShader);
    parkingEntity.setColor({1.0f, 0.0f, 0.0f, 1.0f});
    parkingEntity.setYaw(parkingEnv.setParkingYaw());
    parkingEntity.setWidth(PARKING_LENGTH);
    parkingEntity.setLength(PARKING_WIDTH);
    parkingEntity.setPos(randParkingPos);

    Entity wheelFL(&quad, &rectShader), wheelFR(&quad, &rectShader), wheelRL(&quad, &rectShader), wheelRR(&quad, &rectShader);

    const float wheelWidth = vehicleParams.wheel.width;
    const float wheelLength = vehicleParams.wheel.length;
    for (Entity* wheel : {&wheelFL, &wheelFR, &wheelRL, &wheelRR}) {
        wheel->setColor({0.4f, 0.4f, 0.4f, 1.0f});
        wheel->setWidth(wheelLength);
        wheel->setLength(wheelWidth);
    }
    

    auto placeWheel = [&](Entity& wheel, float ax, float ay, bool front, 
                          const State& pos, const float& yawDraw, const float& steer) {
        
        const float c = cosf(yawDraw), s = sinf(yawDraw);

        // car-local anchor to world position
        const float wx = pos.x + (c*ax - s*ay);
        const float wy = pos.y + (s*ax + c*ay);

        wheel.setPos({wx, wy});

        if (front) {
            wheel.setYaw(yawDraw + steer);
        } else {
            wheel.setYaw(yawDraw);
        }
    };

    // trajectory line
    std::vector<Entity> trajectoryEntities;
    trajectoryEntities.clear();
    trajectoryEntities.reserve(2000);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // timing
        double now = glfwGetTime();
        double frameDt = now - lastTime;
        lastTime = now;
        accumulator += frameDt;

        // input
        // -----
        processInput(window, action);
    
        clampAccumulator(accumulator, simDt);

        // fixed-step simulation
        while (accumulator >= simDt) {
            // set the privious state
            prevState = vehicleState.pos;
            prevPsi = vehicleState.psi;
            prevDelta = vehicleState.delta;
            
            // update state
            bikeModel.kinematicAct(action, vehicleState, static_cast<float>(simDt));
            
            // set the current state
            curState = vehicleState.pos;
            curPsi = vehicleState.psi;
            curDelta  = vehicleState.delta;

            keepOnScreenMeters(vehicleState.pos, carEntity.getWidth(), carEntity.getLength(),fbW, fbH, PPM);
            accumulator -= simDt;
        }

        // interpolate for smooth rendering
        float alpha = static_cast<float>(accumulator / simDt);
        const State posDraw = interp(prevState, curState, alpha);
        const float yawDraw = lerpAngle(prevPsi, curPsi, alpha);
        const float deltaDraw = prevDelta + (curDelta - prevDelta) * alpha;

        // set pos and yaw to draw the car
        carEntity.setPos(posDraw);
        carEntity.setYaw(yawDraw);

        // trajectory
        // calculate the distance between two points
        float dx = curState.x - prevState.x;
        float dy = curState.y - prevState.y;
        float len = std::sqrt(dx*dx + dy*dy);
        
        // ignore small movement
        const float minSegLen = 0.01f;   // 1 cm
        if (len > minSegLen) {
            // center of the segment
            State center{
                0.5f * (prevState.x + curState.x),
                0.5f * (prevState.y + curState.y)
            };

        // yaw of the segment
        float segYaw = std::atan2(dy, dx);

        Entity seg(&quad, &rectShader);
        seg.setPos(center);
        seg.setWidth(0.05f);
        seg.setLength(len);
        seg.setYaw(segYaw);
        seg.setColor({0.9f, 0.9f, 0.2f, 1.0f});

        trajectoryEntities.push_back(seg);

        };

        // check parking success
        bool parkingSuccess = parkingEnv.isParked({carEntity.getPosX(), carEntity.getPosY()}, carEntity.getYaw(), {parkingEntity.getPosX(), parkingEntity.getPosY()}, parkingEntity.getYaw());

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw entities
        renderer.draw(parkingEntity);
        renderer.draw(carEntity);

        placeWheel(wheelFL, anchors[0][0], anchors[0][1], true, posDraw, yawDraw, deltaDraw);
        placeWheel(wheelFR, anchors[1][0], anchors[1][1], true, posDraw, yawDraw, deltaDraw);
        placeWheel(wheelRR, anchors[2][0], anchors[2][1], false, posDraw, yawDraw, 0.0f);
        placeWheel(wheelRL, anchors[3][0], anchors[3][1], false, posDraw, yawDraw, 0.0f);
        
        renderer.draw(wheelFL);
        renderer.draw(wheelFR);
        renderer.draw(wheelRR);
        renderer.draw(wheelRL);

        // draw trajectory
        for (auto& seg : trajectoryEntities) {
            renderer.draw(seg);
        }
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    // carLoader.~Loader(); // call destructor explicitly to delete VBO, VAO, EBO
    // carShader.~CarShader(); // call destructor explicitly to delete shader program


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, Action& action)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // reset action
    action = {};

    // Move a rectangle based on inputs as a discrete action space for simplicity
    // a continuous action space will be implemented later.
    const float iSteeringAngle = PI * 0.166f;  // about 30 degrees
    const float iAcceleration = 1.0f;    // 1 m/s

    // combined actions (e.g. accelerate + steer) are possible
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) action.steeringAngle = -iSteeringAngle;
    if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) action.steeringAngle = +iSteeringAngle;
    if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) action.acceleration = iAcceleration; 
    if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) action.acceleration = -iAcceleration;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


/*
NDC is abbeviated to Normalized Device Coordinates.
If you want all the vertices to become visible, a clip process is needed between -1 and +1 after each vertex shader runs.
Coordinates outside this range is not visible.  
The coordinate for the center of the screen is (0, 0) and top is 1 and bottom is -1 on Y axis, right is +1 and left is -1 on X axis.

Transforming coordinates to NDC is normally achieved in a step-by-step fashin where we transform an object's vertices to several coordinate systems before transforming them to NDC.
There are a total of 5 different coordinate systems that are of essential to you.
- Local space or Object space
- World space
- View space or Eye space
- Clip space
- Screen space

TODO: Research these five coordinate systems to deepen my understanding.

*/

