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


// Global (or file-scope) RNG – constructed once
static std::mt19937 g_rng{ std::random_device{}() };

// simulation state
// simulation state stays as meters

const float PPM = 20.f;  // 1 pixel is 0.05 m (5 cm)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Action& action);

// settings
// window size
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// rectangle as a car size in meters
const float CAR_WIDTH = 2.0f;
const float CAR_HEIGHT = 4.0f;

// rectangle as a parking lot size in meters
const float PARKING_WIDTH = 4.0f;
const float PARKING_HEIGHT = 8.0f;

// wheels
struct WheelSize { float length{0.75f}, width{0.35f}; };
struct VehicleParams {
    // car body
    float carWid{CAR_WIDTH}, carLen{CAR_HEIGHT};

    // wheel
    WheelSize wheel{0.75f, 0.35f};

    // margins
    float front_margin{0.20f}, rear_margin{0.20f}, side_margin{0.10f};

    // wheel placement
    float Lf{};     // front wheel
    float Lr{};     // rear wheel
    float track{};  // wheel centers

    void finalize() {
        Lf = (carLen * 0.5f) - (wheel.length * 0.5f + front_margin);
        Lr = (carLen * 0.5f) - (wheel.length * 0.5f + rear_margin);
        track = carWid - (wheel.width + 2.0f * side_margin);
    }
};

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
inline void keepOnScreenMeters(State& s, float width_m, float height_m, int fbW, int fbH, float ppm) {
    const float worldHalfW = (fbW / ppm) * 0.5f;
    const float worldHalfH = (fbH / ppm) * 0.5f;
    const float marginX = width_m * 0.5f;
    const float marginY = height_m * 0.5f;
    s.x = std::clamp(s.x, -worldHalfW + marginX, worldHalfW - marginX);
    s.y = std::clamp(s.y, -worldHalfH + marginY, worldHalfH - marginY);
};

// angle helpers
// PI is defined in BicycleModel.h
inline float wrapPi(float a){ while(a<=-PI)a+=2*PI; while(a>PI)a-=2*PI; return a; }
inline float lerpAngle(float a,float b,float t){ float d=wrapPi(b-a); return wrapPi(a + d*t); }

// return a random float number in [minVal, maxVal)
// ------------------------------------------------------------------------
inline float randFloat(const float& minVal, const float& maxVal) {
    std::uniform_real_distribution<float> dist(minVal, maxVal);
    return dist(g_rng);
}

// return int in [minVal, maxVal)
// ------------------------------------------------------------------------
inline int randInt(const float& minVal, const float& maxVal) {
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return dist(g_rng);
}

// set the parking lot location randomly
// ------------------------------------------------------------------------
inline State setParkingPos(const float minX, const float maxX, const float minY, const float maxY) {
    float x = randFloat(minX, maxX);
    float y = randFloat(minY, maxY);
    return {x, y};
};

// return yaw either 0 or 90 degree
// ------------------------------------------------------------------------
inline float setParkingYaw() {
    const int k = randInt(0, 1);  // Currently yaw degree shall be 0 or 90 degree
    float yawDeg = (k == 0) ? 0.0f : 90.0f;
    return yawDeg * PI / 180.0f;
}

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

    // entities
    Entity carEntity(&quad, &rectShader);
    carEntity.setColor({0.15f, 0.65f, 0.15f, 1.0f});
    carEntity.setYaw(vehicleState.psi);
    carEntity.setWidth(CAR_HEIGHT);
    carEntity.setHeight(CAR_WIDTH);

    Entity parkingEntity(&quad, &rectShader);
    parkingEntity.setColor({1.0f, 0.8f, 0.2f, 1.0f});
    parkingEntity.setYaw(setParkingYaw());
    parkingEntity.setWidth(PARKING_WIDTH);
    parkingEntity.setHeight(PARKING_HEIGHT);
    parkingEntity.setPos(setParkingPos(-15.f, 15.f, -10.f, 10.f));

    Entity wheelFL(&quad, &rectShader), wheelFR(&quad, &rectShader), wheelRL(&quad, &rectShader), wheelRR(&quad, &rectShader);

    const float wheelWidth = vehicleParams.wheel.width;
    const float wheelLength = vehicleParams.wheel.length;
    for (Entity* wheel : {&wheelFL, &wheelFR, &wheelRL, &wheelRR}) {
        wheel->setColor({0.4f, 0.4f, 0.4f, 1.0f});
        wheel->setWidth(wheelLength);
        wheel->setHeight(wheelWidth);
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

            keepOnScreenMeters(vehicleState.pos, carEntity.getWidth(), carEntity.getHeight(),fbW, fbH, PPM);
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

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { action.acceleration = 0.0f, action.steeringAngle = -iSteeringAngle; } 
    if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) { action.acceleration = 0.0f, action.steeringAngle = +iSteeringAngle; }
    if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) { action.acceleration = iAcceleration, action.steeringAngle = 0.0f; }
    if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) { action.acceleration = -iAcceleration, action.steeringAngle = 0.0f; }
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

