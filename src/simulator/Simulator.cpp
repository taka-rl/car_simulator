
#include "Simulator.h"


namespace {
    // Unit quad in NDC-space centered at origin
    float QUAD_VERTICES[] = {
        0.5f,  0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
       -0.5f,  0.5f, 0.0f
    };

    unsigned int QUAD_INDICES[] = {
        0, 1, 3,
        1, 2, 3
    };

    constexpr std::size_t QUAD_VERTEX_COUNT = sizeof(QUAD_VERTICES) / sizeof(float);
    constexpr std::size_t QUAD_INDEX_COUNT  = sizeof(QUAD_INDICES)  / sizeof(unsigned int);
}


// constructor
Simulator::Simulator(GLFWwindow* window) : 
    window(window), randomizer(), env(&randomizer),  
    carEntity(quad.get(), rectShader.get()), parkingEntity(quad.get(), rectShader.get()), 
    wheelFL(quad.get(), rectShader.get()), wheelFR(quad.get(), rectShader.get()),
    wheelRL(quad.get(), rectShader.get()), wheelRR(quad.get(), rectShader.get()) {};

bool Simulator::init() {
    initRenderer();
    std::cout << "Init Render done" << std::endl;
    initSimulationState();
    std::cout << "Init Simulation State done" << std::endl;
    initEntities();
    std::cout << "Init Entity done" << std::endl;
    return true;
}

void Simulator::initRenderer() {
    // after gladLoadGLLoader(...)
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);
    // (optional) also trigger your callback once to keep all logic in one place:
    framebuffer_size_callback(window, fbW, fbH);
    // build and compile our shader program
    // ------------------------------------
    
    rectShader = std::make_unique<RectShader>();
    
    // set up vertex data (and buffer(s)) and configure vertex attributes
    quad = std::make_unique<Loader>(QUAD_VERTICES, QUAD_VERTEX_COUNT, QUAD_INDICES,  QUAD_INDEX_COUNT);

    // renderer
    renderer = std::make_unique<Renderer>(PPM, fbW, fbH);
}

// initialize simulation state: env, vehicle params
// ------------------------------------------------------------------------
void Simulator::initSimulationState() {
    // reset the environment
    env.reset();
    
    // wheels
    vehicleParams.finalize();

    // wheel anchors in car-local frame
    anchors = {{
        {+vehicleParams.Lf, +vehicleParams.track*0.5f}, 
        {+vehicleParams.Lf, -vehicleParams.track*0.5f},
        {-vehicleParams.Lr, -vehicleParams.track*0.5f}, 
        {-vehicleParams.Lr, +vehicleParams.track*0.5f}
    }};

    // previous and current state for interpolation
    prevState = env.getVehicleState().pos;
    prevPsi = env.getVehicleState().psi;
    curState = env.getVehicleState().pos;
    curPsi = env.getVehicleState().psi;
    curDelta = env.getVehicleState().delta;
}

// initialize entities: car, parking lot, wheels and trajectory
// ------------------------------------------------------------------------
void Simulator::initEntities() {   
    // entities
    const VehicleState vehicleState = env.getVehicleState();
    carEntity = Entity(quad.get(), rectShader.get());
    carEntity.setColor({0.15f, 0.65f, 0.15f, 1.0f});
    carEntity.setYaw(vehicleState.psi);
    carEntity.setWidth(CAR_LENGTH);
    carEntity.setLength(CAR_WIDTH);
    carEntity.setPos(vehicleState.pos);

    const Position2D parkingPos = env.getParkingPos();
    const float parkingYaw = env.getParkingYaw();
    parkingEntity = Entity(quad.get(), rectShader.get());
    parkingEntity.setColor({1.0f, 0.0f, 0.0f, 1.0f});
    parkingEntity.setYaw(parkingYaw);
    parkingEntity.setWidth(PARKING_LENGTH);
    parkingEntity.setLength(PARKING_WIDTH);
    parkingEntity.setPos(parkingPos);

    wheelFL = Entity(quad.get(), rectShader.get()), wheelFR = Entity(quad.get(), rectShader.get());
    wheelRL = Entity(quad.get(), rectShader.get()), wheelRR = Entity(quad.get(), rectShader.get());

    const float wheelWidth = vehicleParams.wheel.width;
    const float wheelLength = vehicleParams.wheel.length;
    for (Entity* wheel : {&wheelFL, &wheelFR, &wheelRL, &wheelRR}) {
        wheel->setColor({0.4f, 0.4f, 0.4f, 1.0f});
        wheel->setWidth(wheelLength);
        wheel->setLength(wheelWidth);
    }
    
    // trajectory line
    trajectoryEntities.clear();
    trajectoryEntities.reserve(2000);
}

void Simulator::placeWheel(Entity& wheel, float ax, float ay, bool front, 
                      const Position2D& pos, const float& yawDraw, const float& steer) {
    
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

void Simulator::run() {
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {      
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
        tick();

        // draw including interpolation factor
        draw();
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}

// step the simulation with fixed time step
// ------------------------------------------------------------------------
void Simulator::tick() {
    while (accumulator >= simDt) {
        // set the privious Position2D
        prevState = curState;
        prevPsi = curPsi;
        prevDelta = curDelta;
        
        // update Position2D
        const float dt = static_cast<float>(simDt);
        Observation obs = env.step(action, dt);

        // set the current Position2D
        curState = obs.vehicleState.pos;
        curPsi = obs.vehicleState.psi;
        curDelta  = obs.vehicleState.delta;

        // TODO: keepOnScreenMeters does not work well. Fix it later.
        keepOnScreenMeters(obs.vehicleState.pos, carEntity.getWidth(), carEntity.getLength(),fbW, fbH, PPM);
        accumulator -= simDt;
    }
}

// draw all entities including interpolation
// ------------------------------------------------------------------------
void Simulator::draw() {
    // interpolate for smooth rendering
    float alpha = static_cast<float>(accumulator / simDt);
    const Position2D posDraw = interp(prevState, curState, alpha);
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
        Position2D center{
            0.5f * (prevState.x + curState.x),
            0.5f * (prevState.y + curState.y)
        };

    // yaw of the segment
    float segYaw = std::atan2(dy, dx);

    Entity seg(quad.get(), rectShader.get());
    seg.setPos(center);
    seg.setWidth(0.05f);
    seg.setLength(len);
    seg.setYaw(segYaw);
    seg.setColor({0.9f, 0.9f, 0.2f, 1.0f});

    trajectoryEntities.push_back(seg);

    };

    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw entities
    renderer->draw(parkingEntity);
    renderer->draw(carEntity);

    placeWheel(wheelFL, anchors[0][0], anchors[0][1], true, posDraw, yawDraw, deltaDraw);
    placeWheel(wheelFR, anchors[1][0], anchors[1][1], true, posDraw, yawDraw, deltaDraw);
    placeWheel(wheelRR, anchors[2][0], anchors[2][1], false, posDraw, yawDraw, 0.0f);
    placeWheel(wheelRL, anchors[3][0], anchors[3][1], false, posDraw, yawDraw, 0.0f);
        
    renderer->draw(wheelFL);
    renderer->draw(wheelFR);
    renderer->draw(wheelRR);
    renderer->draw(wheelRL);

    // draw trajectory
    for (auto& seg : trajectoryEntities) {
        renderer->draw(seg);
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Simulator::processInput(GLFWwindow *window, Action& action) {
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
void Simulator::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Get the simulator instance from the window user pointer
    Simulator* sim = static_cast<Simulator*>(glfwGetWindowUserPointer(window));
    if (sim) {
        sim->fbW = width;
        sim->fbH = height;
        glViewport(0, 0, width, height);
    }
}
