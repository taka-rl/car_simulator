# Class Architecture – Car Simulator
This document explains **how the major classes fit together**, what each layer owns, and the key invariants (especially around OpenGL context/lifetime).  
For the full, exhaustive list of methods/attributes per class, see `class_diagram.md`.

---

## Architecture at a glance

### Layers

1. **Application / Platform**
   - `Window` (GLFW + GLAD + OpenGL context lifetime)
   - `main.cpp` (creates `Window`, then starts `Simulator`)

2. **Simulation Orchestrator**
   - `Simulator` 
      - Input produces `Action`
      - `ParkingEnv::step(Action, dt)` updates `VehicleState` and produces `Observation`
      - `Simulator` stores prev/cur snapshots
      - `draw()` interpolates and renders

3. **Environment (Parking task)**
   - `ParkingEnv` (step the environment by one time step, parking slot placement, termination checks, reward computation, reset the environment)
   - `ParkingParams` (success tolerances)

4. **Vehicle Dynamics**
   - `BicycleModel` (kinematic bicycle update)
   - `VehicleTypes` (`VehicleState`, `VehicleParams`, `Action`, `Position2D`)
   - `MathUtils` (angle helpers / constants)

5. **Rendering (OpenGL rectangles)**
   - `Renderer` (meters → NDC conversion, draw calls)
   - `Entity` (render instance: pose/size/color + links to mesh/shader)
   - `Loader` (unit quad mesh: VAO/VBO/EBO)
   - `RectShader` → `ShaderProgram` (shader program + cached uniform locations)

6. **Utilities**
   - `Randomizer` (RNG utilities used by `ParkingEnv`)

---

## Dependency rules

- **Pure math / types** (`VehicleTypes`, `MathUtils`, `ParkingParams`) must not depend on OpenGL/GLFW.
- **Dynamics / env** (`BicycleModel`, `ParkingEnv`) should stay OpenGL-free.
- Only the **rendering layer** (`Renderer`, `Loader`, `ShaderProgram`, `RectShader`) touches OpenGL.
- Any creation of RectShader/Loader/Renderer must happen after `Window` has created the context + loaded GLAD.
- `Entity` should not own GPU resources; it should reference shared render resources.
- `Window` owns the GLFWwindow; Simulator only borrows GLFWwindow*. Therefore `Window` must outlive `Simulator`.
---

## Initialization and main loop flow
1. main.cpp
   - Window window(...)
      - calls glfwInit (once), creates GLFWwindow, makes context current, loads GLAD, sets vsync.
   - Simulator sim(window.get()) (stores raw GLFWwindow*)

2. Simulator::init()
   - initRenderer()
      - creates RectShader, Loader(quad), Renderer, sets viewport (fbW/fbH)
   - initSimulationState()
      - env.reset()
      - vehicleParams.finalize(): computes wheel anchors
      - sets prev/cur state, lastTime/accumulator
   - initEntities()
      - constructs car/parking/wheels/entities using the created render resources

3. Simulator::run()
   - per frame
      - processInput()
      - accumulate dt
      - tick() (while accumulator >= simDt: env.step)
      - draw() (interpolation)
      - swap/poll

## Sequence plot for initialization and main loop

```mermaid
sequenceDiagram
  autonumber
  participant main
  participant Win as Window
  participant GLFW as GLFW/GLAD/OpenGL
  participant Sim as Simulator
  participant Env as ParkingEnv
  participant BM as BicycleModel
  participant Ren as Renderer

  main->>Win: Window(width, height, title)
  Win->>GLFW: glfwInit() (once via static flag)
  Win->>GLFW: glfwWindowHint(GL 3.3 core)
  Win->>GLFW: glfwCreateWindow(...)
  Win->>GLFW: glfwMakeContextCurrent(window)
  Win->>GLFW: gladLoadGLLoader(glfwGetProcAddress)
  Win->>GLFW: glfwSwapInterval(1)

  main->>Win: isValid()
  main->>Sim: Simulator(Win.get())
  main->>Sim: init()

  Sim->>Sim: initRenderer()
  Sim->>GLFW: glfwGetFramebufferSize(window,&fbW,&fbH)
  Sim->>GLFW: glViewport(0,0,fbW,fbH)
  Sim->>Sim: framebuffer_size_callback(window, fbW, fbH)
  Sim->>Sim: rectShader = make_unique<RectShader>()
  Sim->>Sim: quad = make_unique<Loader>(QUAD_*)
  Sim->>Ren: renderer = make_unique<Renderer>(PPM, fbW, fbH)

  Sim->>Sim: initSimulationState()
  Sim->>Env: reset()
  Env->>Env: setParkingPos(), setParkingYaw()
  Env->>Env: init vehicleState (random pos/yaw, v=0, delta=0)
  Sim->>Sim: vehicleParams.finalize()
  Sim->>Sim: anchors setup (Lf/Lr/track)
  Sim->>Sim: prev/cur state init from Env.getVehicleState()
  Sim->>GLFW: lastTime = glfwGetTime()
  Sim->>Sim: accumulator = 0

  Sim->>Sim: initEntities()
  Sim->>Sim: car/parking/wheels Entities created using (quad, rectShader)

  main->>Sim: run()
  loop each frame until glfwWindowShouldClose
    Sim->>GLFW: now = glfwGetTime()
    Sim->>GLFW: frameDt = now - lastTime
    Sim->>GLFW: lastTime = now
    Sim->>Sim: processInput(window, action)
    Sim->>Sim: clampAccumulator(accumulator, simDt)

    Sim->>Sim: tick()
    loop while accumulator >= simDt
      Sim->>Env: step(action, dt)
      Env->>BM: kinematicAct(action, vehicleState, dt)
      Env->>Env: reward()
      Env->>Env: calculateRelCorners(...)
      Env-->>Sim: Observation{distCorners, vehicleState}
      Sim->>Sim: update prev/cur
      Sim->>Sim: accumulator -= simDt
    end

    Sim->>Sim: draw() (interpolate alpha)
    Sim->>Ren: drawEntities(...)
    Sim->>GLFW: glfwSwapBuffers(window)
    Sim->>GLFW: glfwPollEvents()
  end
```