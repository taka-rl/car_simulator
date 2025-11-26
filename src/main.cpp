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


// Global (or file-scope) RNG – constructed once
static std::mt19937 g_rng{ std::random_device{}() };

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

// angle helpers
// PI is defined in BicycleModel.h
inline float wrapPi(float a){ while(a<=-PI)a+=2*PI; while(a>PI)a-=2*PI; return a; }
inline float lerpAngle(float a,float b,float t){ float d=wrapPi(b-a); return wrapPi(a + d*t); }

// return a random float number in [minVal, maxVal)
// ------------------------------------------------------------------------
inline float randFloat(float minVal, float maxVal) {
    std::uniform_real_distribution<float> dist(minVal, maxVal);
    return dist(g_rng);
}

// return int in [minVal, maxVal)
// ------------------------------------------------------------------------
inline int randInt(int minVal, int maxVal) {
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return dist(g_rng);
}

// set the parking lot location randomly
// ------------------------------------------------------------------------
inline State setParkingPos(float minX, float maxX, float minY, float maxY) {
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

// rotate car poistion into the parking lot frame
// ------------------------------------------------------------------------
inline State worldToSlot(const State& carPos, const State& slotPos, float slotYaw) {
    const float dx = carPos.x - slotPos.x;
    const float dy = carPos.y - slotPos.y;

    const float c = cosf(slotYaw);
    const float s = sinf(slotYaw);

    return State{
        c * dx + s * dy,
        -s * dx + c * dy
    };
}

// Parking success tolerances (in slot frame)
constexpr float PARK_LONG_TOL = 1.5f;   // ±1.5 m along slot axis on X axis
constexpr float PARK_LAT_TOL  = 1.f;    // ±1.0 m sideways on Y axis
constexpr float PARK_YAW_TOL  = 10.0f * (PI / 180.0f); // 10 deg in rad

/**
 * @brief Check if the car is roughly centered and aligned in the parking slot (slot-frame check).
 *
 * This is a *soft* parking check used mainly for shaping rewards.
 * It works entirely in the parking slot frame:
 *
 *  1. Transform the car center from world frame into the parking slot frame
 *     using worldToSlot(carPos, parkingPos, parkingYaw).
 *  2. Compute the relative heading error psiRel = wrapPi(carYaw - parkingYaw).
 *  3. Apply simple tolerances on position and yaw:
 *       - |rel.x| <= PARK_LONG_TOL   (along slot axis / length direction)
 *       - |rel.y| <= PARK_LAT_TOL    (sideways within the slot)
 *       - |psiRel| <= PARK_YAW_TOL   (heading aligned with slot)
 *
 * Currently the function only enforces the position tolerance (posOk) and
 * ignores yawOk in the returned result, but yawOk is computed and logged and
 * can be enabled later (for example, for RL reward shaping).
 *
 * @param carPos      Car center position in world frame [meters].
 * @param carYaw      Car heading in world frame [radians, CCW+, x-forward].
 * @param parkingPos  Parking slot center in world frame [meters].
 * @param parkingYaw  Parking slot orientation in world frame [radians].
 *
 * @return true if the car center lies within the configured longitudinal and
 *         lateral tolerances of the parking slot center (posOk).
 *         false otherwise.
 */
inline bool isParkedAtCenter(const State& carPos, const float carYaw, const State& parkingPos, const float& parkingYaw) {

    // car center in slot frame
    State rel = worldToSlot(carPos, parkingPos, parkingYaw);

    // heading error in slot frame
    const float psiRel = wrapPi(carYaw - parkingYaw);
    
    // position tolerances (slot frame)
    const bool posOk = std::fabs(rel.x) <= PARK_LONG_TOL && std::fabs(rel.y) <= PARK_LAT_TOL;

    // yaw tolerance
    const bool yawOk = std::fabs(psiRel) <= PARK_YAW_TOL;

    // debug
    // std::cout << "Slot frame: rel.x=" << rel.x
    // << " rel.y=" << rel.y
    // << " |psiRel|=" << std::fabs(psiRel)
    // << " |yaw|=" << PARK_YAW_TOL
    // << " posOk=" << posOk
    // << " yawOk=" << yawOk << "\n";

    // temporarily only position is used to check parking success
    // if (posOk && yawOk) {
    if (posOk) {
        std::cout << "Car is at the center of the parking lot!" << std::endl;
    } else {
        std::cout << "Not at the center of the parking lot" << std::endl;
    }
    
    return posOk; //&& yawOk; 
}

/**
 * @brief Strict geometric parking check: full rotated car rectangle must lie inside the rotated parking lot rectangle.
 * 
 * This function performs an exact 2D rectangle-in-rectangle test in the parking lot frame.
 * 
  1. Interpret CAR_LENGTH as car length (along car local x: forward)
 *     and CAR_WIDTH as car width (along car local y: left). The parking slot
 *     uses PARKING_LENGTH as slot length and PARKING_WIDTH as slot width.
 *
 *  2. Define the parking-slot frame:
 *       - Origin at parkingPos
 *       - X-axis along parkingYaw (slot length direction)
 *       - Y-axis to the left of X (slot width direction)
 *
 *  3. Transform the car center from world frame into the slot frame:
 *       rel = worldToSlot(carPos, parkingPos, parkingYaw)
 *
 *  4. Compute the car orientation relative to the slot:
 *       psiRel = wrapPi(carYaw - parkingYaw)
 *       (cRel, sRel) = (cos(psiRel), sin(psiRel))
 *
 *  5. Construct the four car corners in car-local frame:
 *       (±halfCarLen, ±halfCarWid)
 *     and transform each corner into slot frame via:
 *
 *       [x']   [  cRel  -sRel ] [local.x] + rel.x
 *       [y'] = [  sRel   cRel ] [local.y] + rel.y
 *
 *  6. For each transformed corner (x', y'), check that it lies within the
 *     slot half-extent:
 *
 *       |x'| <= halfSlotLen  &&  |y'| <= halfSlotWid
 *
 *     If any corner violates this, the car overlaps the slot boundary and
 *     the function returns false.
 *
 * Because both car and slot are handled in arbitrary orientations, this
 * works for 0°, 90°, 180°, 270° slots and any car heading in [-π, π].
 *
 * @param carPos      Car center position in world frame [meters].
 * @param carYaw      Car heading in world frame [radians, CCW+, x-forward].
 * @param parkingPos  Parking slot center in world frame [meters].
 * @param parkingYaw  Parking slot orientation in world frame [radians].
 *
 * @return true if all four car corners are inside the parking slot rectangle
 *         in the slot frame; false otherwise.
 */
inline bool isParked(const State& carPos, float carYaw, const State& parkingPos, float parkingYaw) {

    // This code will be used for RL
    // return false if the car is not at the center of the parking lot
    // if (!(isParkedAtCenter(carPos, carYaw, parkingPos, parkingYaw))) {
    //     return false;
    // }
    
    // calculate half sizes (meters)
    const float halfCarLen = CAR_LENGTH * 0.5f;       // along car local x (forward)
    const float halfCarWid = CAR_WIDTH  * 0.5f;       // along car local y (left)
    const float halfSlotLen = PARKING_LENGTH * 0.5f;  // along slot local X
    const float halfSlotWid = PARKING_WIDTH  * 0.5f;  // along slot local Y

    // Car center in slot frame
    // Define slot frame: origin at parkingPos, X along parkingYaw, Y left of X
    const float dx = carPos.x - parkingPos.x;
    const float dy = carPos.y - parkingPos.y;

    const float cSlot = std::cos(parkingYaw);
    const float sSlot = std::sin(parkingYaw);

    // Rotate world -> slot frame
    // [ X_slot ]   [  cos  sin ] [ dx ]
    // [ Y_slot ] = [ -sin  cos ] [ dy ]
    State rel;
    rel.x =  cSlot * dx + sSlot * dy;  // along slot length
    rel.y = -sSlot * dx + cSlot * dy;  // along slot width

    // Car orientation relative to slot
    const float psiRel = wrapPi(carYaw - parkingYaw);
    const float cRel   = std::cos(psiRel);
    const float sRel   = std::sin(psiRel);

    // Car corners in *car* local frame (x forward, y left)
    // Four corners: (±halfLen, ±halfWid)
    std::array<State, 4> carLocalCorners = {{
        { +halfCarLen, +halfCarWid },
        { +halfCarLen, -halfCarWid },
        { -halfCarLen, -halfCarWid },
        { -halfCarLen, +halfCarWid }
    }};

    // Transform each car corner into slot frame and test
    for (const auto& local : carLocalCorners) {
        // Rotate corner from car frame -> slot frame
        // [x']   [  cRel  -sRel ] [local.x]
        // [y'] = [  sRel   cRel ] [local.y]
        const float cx = local.x;
        const float cy = local.y;

        const float vx = rel.x + (cRel * cx - sRel * cy);
        const float vy = rel.y + (sRel * cx + cRel * cy);

        const bool insideX = std::fabs(vx) <= halfSlotLen;
        const bool insideY = std::fabs(vy) <= halfSlotWid;

        if (!insideX || !insideY) {
            // at least one corner is outside → not parked
            std::cout << "Not parked" << std::endl;
            return false;
        }
    }

    // all 4 corners are inside the slot box in slot frame → parked
    std::cout << "Parked" << std::endl;
    return true;
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

    // random positions for car and parking
    const State randParkingPos = setParkingPos(-15.f, 15.f, -10.f, 10.f);
    const float marginX = randFloat(-5.0, 5.0), marginY = randFloat(-5.0, 5.0);
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
    parkingEntity.setYaw(setParkingYaw());
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
        bool parkingSuccess = isParked({carEntity.getPosX(), carEntity.getPosY()}, carEntity.getYaw(), {parkingEntity.getPosX(), parkingEntity.getPosY()}, parkingEntity.getYaw());

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

