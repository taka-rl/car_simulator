#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <algorithm>
#include <array>
#include <cmath>

#include "shaders/RectShader.h"
#include "Loader.h"
#include "entities/Entity.h"
#include "renderers/Renderer.h"


// simulation state
// simulation state stays as meters

const float PPM = 20;  // 1 pixel is 0.05 m (5 cm)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float& ix, float& iy);
void step(State& prevState, State& curState, const double& simDt, float& ix, float& iy);

// settings
// window size
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// rectangle as a car size in meters
const float CAR_WIDTH = 2.0;
const float CAR_HEIGHT = 4.0;

// rectangle as a parking lot size in meters
const float PARKING_WIDTH = 4.0;
const float PARKING_HEIGHT = 8.0;

// wheels
struct WheelSize { float length{0.75}, width{0.35}; };
struct VehicleParams {
    float Lf{0.75}, Lr{0.75}, track{2.5};
    WheelSize wheel{0.75, 0.35};
    float carWid{CAR_WIDTH}, carLen{CAR_HEIGHT};
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

    // simulation state (previous and current for interpolation)
    State prevState{0,0}, curState{0,0};
    float yaw = 0.5f; // about 30 degrees

    // inputs
    float ix = 0.0f, iy = 0.0f;

    // wheels
    VehicleParams vehicleParams;

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
    carEntity.setYaw(yaw);
    carEntity.setWidth(CAR_WIDTH);
    carEntity.setHeight(CAR_HEIGHT);

    Entity parkingEntity(&quad, &rectShader);
    parkingEntity.setColor({1.0f, 0.8f, 0.2f, 1.0f});
    parkingEntity.setYaw(0.0f);
    parkingEntity.setWidth(PARKING_WIDTH);
    parkingEntity.setHeight(PARKING_HEIGHT);
    parkingEntity.setPos({2.0f, 2.0f});  // position is fixed temporarily

    Entity wheelFL(&quad, &rectShader), wheelFR(&quad, &rectShader), wheelRL(&quad, &rectShader), wheelRR(&quad, &rectShader);

    const float wheelWidth = vehicleParams.wheel.width;
    const float wheelLength = vehicleParams.wheel.length;
    for (Entity* wheel : {&wheelFL, &wheelFR, &wheelRL, &wheelRR}) {
        wheel->setColor({0.4f, 0.4f, 0.4f, 1.0f});
        wheel->setWidth(wheelWidth);
        wheel->setHeight(wheelLength);
    }
    auto placeWheel = [&](Entity& wheel, float ax, float ay, bool front) {
        const float yaw = carEntity.getYaw();
        const float c = cosf(yaw), s = sinf(yaw);

        // car-local anchor to world position
        const float wx = carEntity.getPosX() + (c*ax - s*ay);
        const float wy = carEntity.getPosY() + (s*ax + c*ay);

        wheel.setPos({wx, wy});

        // Later for front wheels adding steering angle is possible
        wheel.setYaw(yaw);
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
        processInput(window, ix, iy);
    
        clampAccumulator(accumulator, simDt);

        // fixed-step simulation
        while (accumulator >= simDt) {
            step(prevState, curState, simDt, ix, iy);
            keepOnScreenMeters(curState, carEntity.getWidth(), carEntity.getHeight(),fbW, fbH, PPM);
            accumulator -= simDt;
        }

        // interpolate for smooth rendering
        float alpha = static_cast<float>(accumulator / simDt);
        // State drawS = interp(prevState, curState, alpha);
        carEntity.setPos(interp(prevState, curState, alpha));

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw entities
        renderer.draw(parkingEntity);
        renderer.draw(carEntity);

        placeWheel(wheelFL, anchors[0][0], anchors[0][1], true);
        placeWheel(wheelFR, anchors[1][0], anchors[1][1], true);
        placeWheel(wheelRR, anchors[2][0], anchors[2][1], false);
        placeWheel(wheelRL, anchors[3][0], anchors[3][1], false);
        
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
void processInput(GLFWwindow *window, float& ix, float& iy)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Move a rectangle based on inputs
    float dx = 0.0f, dy = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) dx += 1.0;
    if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) dx -= 1.0;
    if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) dy += 1.0;
    if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) dy -= 1.0;

    // simple critically-damped-ish smoothing toward target inputs (optional)
    // makes input changes less jittery between frames
    const float k = 0.25f; // smoothing factor [0..1], 0=no change, 1=instant
    ix = ix + k * (dx - ix);
    iy = iy + k * (dy - iy);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// simulation step
// ---------------------------------------------------------------------------------------------------------
void step(State& prevState, State& curState, const double& simDt, float& ix, float& iy) {
    // simple kinematic “speed” in NDC units per second
    const float SPEED_MS = 1.0f;  // 1 m/s

    // shift previous → current for interpolation
    prevState = curState;

    // apply input as velocity command
    curState.x += (ix * SPEED_MS) * static_cast<float>(simDt);
    curState.y += (iy * SPEED_MS) * static_cast<float>(simDt);
};


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

