#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <algorithm>

#include "shaders/RectShader.h"
#include "Loader.h"


// simulation state
// simulation state stays as meters
struct State { float x, y; };
const float PPM = 20;  // 1 pixel is 0.05 m (5 cm)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float& ix, float& iy);
void step(State& prevState, State& curState, const double& simDt, float& ix, float& iy);
void keepQuadNDC(State& prevState, State& curState) ;


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

// meters to NDC
inline State metersToNDC(float x_m, float y_m, int fbW, int fbH, float ppm) {
    const float Wm = fbW / ppm, Hm = fbH / ppm;
    return State{
        x_m / (Wm * 0.5f),
        y_m / (Hm * 0.5f)
    };
}

// rect size in NDC (from meters)
inline State rectScaleNDC(float width_m, float height_m, int fbW, int fbH, float ppm) {
    return State{
        2.0f * width_m * ppm / fbW,
        2.0f * height_m * ppm / fbH
    };
};

// keep car on screen with meters
inline void keepCarOnScreenMeters(State& s, int fbW, int fbH, float ppm) {
    const float worldHalfW = (fbW / ppm) * 0.5f;
    const float worldHalfH = (fbH / ppm) * 0.5f;
    const float marginX = CAR_WIDTH  * 0.5f;
    const float marginY = CAR_HEIGHT * 0.5f;
    s.x = std::clamp(s.x, -worldHalfW + marginX, worldHalfW - marginX);
    s.y = std::clamp(s.y, -worldHalfH + marginY, worldHalfH - marginY);
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
    RectShader carShader;
    RectShader parkingShader;

    float vertices[] = {
        0.5,  0.5, 0.0f,  // top right
        0.5,  -0.5, 0.0f,  // bottom right
        -0.5, -0.5, 0.0f,  // bottom left
        -0.5, 0.5, 0.0f   // top left 
        };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    // set up vertex data (and buffer(s)) and configure vertex attributes
    Loader carLoader(vertices, sizeof(vertices)/sizeof(float), indices, sizeof(indices)/sizeof(unsigned int));
    Loader parkingLoader(vertices, sizeof(vertices)/sizeof(float), indices, sizeof(indices)/sizeof(unsigned int));
    
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

    // inputs
    float ix = 0.0f, iy = 0.0f;

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
            keepCarOnScreenMeters(curState, fbW, fbH, PPM);
            accumulator -= simDt;
        }

        // interpolate for smooth rendering
        float alpha = static_cast<float>(accumulator / simDt);
        State drawS = interp(prevState, curState, alpha);
        
        // car
        State s_ndc = rectScaleNDC(CAR_WIDTH, CAR_HEIGHT, fbW, fbH, PPM);
        State ndc = metersToNDC(drawS.x, drawS.y, fbW, fbH, PPM);

        // parking
        State parkingS_ndc = rectScaleNDC(PARKING_WIDTH, PARKING_HEIGHT, fbW, fbH, PPM);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw a parking lot
        parkingShader.use();
        parkingShader.setOffset(0.2f, 0.2f);
        parkingShader.setColor(1.0f, 0.8f, 0.2f, 1.0f);
        parkingShader.setScale(parkingS_ndc.x, parkingS_ndc.y);
        glBindVertexArray(parkingLoader.getVAO());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // draw a car
        carShader.use();
        carShader.setOffset(ndc.x, ndc.y);
        carShader.setColor(0.15f, 0.65f, 0.15f, 1.0f);
        carShader.setScale(s_ndc.x, s_ndc.y);
        glBindVertexArray(carLoader.getVAO()); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        
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

