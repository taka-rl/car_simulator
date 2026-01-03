# Car Simulator -- Dev Notes

## Overview
Top-down 2D car simulator: **car body + 4 wheels**, meters-first physics with a **fixed timestep**, and smooth rendering via a single **unit-quad** mesh and a **RectShader** (scale → rotate → translate).

---
## Simulator Environment

### Environment settings (source of truth)
#### Display and world scaling
Defined in `core/Config.h`:

| Constant | Meaning | Value |
|---|---|---|
| `SCR_WIDTH`, `SCR_HEIGHT` | window size in pixels | 800 × 600 |
| `PPM` | pixels-per-meter, 1 pixel is defined 0.05 m (5 cm). | 20.0 |

**World extents (in meters)** (useful for clamping/spawning):
- half-width: `(SCR_WIDTH / PPM) / 2`
- half-height: `(SCR_HEIGHT / PPM) / 2`

#### Geometry sizes
Also in `core/Config.h`, and `vehicledynamics/VehicleTypes.h`:

| Constant | Meaning | Value (m) |
|---|---|---|
| `CAR_WIDTH`, `CAR_LENGTH` | car rectangle size | 2.0 × 4.0 |
| `PARKING_WIDTH`, `PARKING_LENGTH` | parking slot size | 3.5 × 6.0 |
| `wheel.width=0.35f`, `wheel.length=0.75f` | Wheel geometry (m) | 0.35 m width × 0.75 m length  |

#### Simulation
Defined in `simulator/Simulator.h` and `vehicledynamics/BicycleModel.h`:
| Constant | Meaning | Value |
|---|---|---|
| Simulation step | 0.01s  | `simDt=0.01` |
| Kinematic bicycle limits | acceleration <= 1.0 m/s², steering angle <= 45 degrees,  velocity <= 10km/h ≈ 2.78 m/s  | `a_max=1.0f`, `delta_max=0.785f`, `v_max=2.78f` |

#### Parking success tolerances
Defined in `envs/ParkingParams.h` (slot-frame tolerances):

| Constant | Meaning | Value |
|---|---|---|
| `PARK_LONG_TOL` | along slot axis (x in slot frame) | ±1.5 m |
| `PARK_LAT_TOL` | lateral (y in slot frame) | ±1.0 m |
| `PARK_YAW_TOL` | heading alignment | ±10° |

---

### Units
In this project, **meters, radians and seconds** are used in physics simulations and NDC is used for rendering. 

### Coordinate frames & signs
- **World**: +X right, +Y up.
- **Vehicle local**: x = forward (longitudinal), y = left (lateral).
- **Angles**: yaw (psi) ψ is CCW-positive; steering angle δ (delta) is CCW-positive (left = +δ).

### Rendering

All objects are rectangles rendered from a **shared unit quad**:
- `Loader` creates VAO/VBO/EBO once
- `RectShader` renders rectangles by applying:
  - scale → rotate → translate inside the vertex shader

Draw sequence (per entity): 
1. Convert meters → NDC (offset + scale)
2. Set shader uniforms:
   - `uOffset`, `uScale`, `uYaw`, `uColor`
3. Bind shared VAO
4. `glDrawElements`

#### Mesh (Loader class)
Single **unit quad** centered at (0,0) with vertices at ±0.5; shared VAO/VBO/EBO.

#### Material (RectShader class) 
Uniforms: `uOffset` (NDC center), `uScale` (full NDC size), `uYaw` (CCW), `uColor`.

##### Vertex shader core rules
In `rectShader.vert`, **scale → rotate (CCW) → translate** is applied and **the offset** is not rotated.

#### Entity (Entity class)
It holds **pos_m** (x, y in m), **yaw** (rad), **size** (length, width in m), **color**, pointers to shared **Mesh** and **Material**.

#### Renderer pipeline (2D)
For each entity (car body, wheels, parking rectangle), rendering is executed in Renderer class with Loader, RectShader and Entity classes.
1. Convert meters → NDC with `metersToNDC(x, y)` and `rectSizeToNDC(width, length)`.
2. Set `uOffset`, `uYaw`, `uScale` and `uColor` by accessing RectShader pointer via Entity class.
3. Bind the shared VAO, draw with `glDrawElements`.


---

## Simulation loop

### Fixed timestep with accumulator
The simulator uses a fixed simulation timestep (default `simDt = 0.01s`) and an accumulator:
1. Measure `frameDt = now - lastTime`
2. `accumulator += frameDt`
3. Clamp accumulator to avoid “spiral of death” after stalls
4. While `accumulator >= simDt`:
   - store `prevState`
   - step physics with `simDt`
   - `accumulator -= simDt`

### Interpolated render
Render once per frame using:
- `alpha = accumulator / simDt`
- interpolate **position**, and use angle interpolation for yaw:
  - `posDraw = lerp(prevPos, curPos, alpha)`
  - `yawDraw = lerpAngle(prevYaw, curYaw, alpha)`
  - `steerDraw = lerp(prevDelta, curDelta, alpha)`

This keeps simulation stable and makes rendering smooth.

#### Input Actions
Discrete inputs: **acceleration** and **steering angle**.  
Combined actions (e.g. accelerate + steer) are possible.

---

### Vehicle Dynamics

#### Vehicle geometry & anchors
Car body sizes are 2 m width × 4 m length, and wheel sizes are 0.35 meter width × 0.75 meter length. 
Anchors in car frame (x forward, y left):
- FL ( +Lf, +track/2 ), FR ( +Lf, −track/2 )
- RL ( −Lr, +track/2 ), RR ( −Lr, −track/2 )

Keep wheels visually inside the body (choose margins):  
carLen = CAR_LENGTH, carWid = CAR_WIDTH
carLen=4.0f;  
front_margin = 0.25 m, rear_margin = 0.25 m, side_margin = 0.10 m  
Lf = (carLen * 0.5f) - (wheel.length * 0.5f +front_margin);  
Lr = (carLen * 0.5f) - (wheel.length * 0.5f +rear_margin);  
track = carWid - (wheel.width + 2.0f *side_margin);


#### Kinematic Bicycle Model
From `vehicledynamics/VehicleTypes.h`:

- `VehicleState`:
  - `pos: {x, y}` [m]
  - `psi` [rad]
  - `velocity` [m/s]
  - `delta` [rad]

- `Action`:
  - `acceleration` [m/s²]
  - `steeringAngle` [rad] (current discrete setpoint)

Here is the kinematic bicycle model used in this simulation (With wheelbase `L = Lf + Lr`) 

- x_dot = v * cos(psi)
- y_dot = v * sin(psi)
- v_dot = acceleration
- psi_dot = v * tan(delta) / L
psi_dot is clipped between -pi/2 and +pi/2. 

One step forward:
- x_(t+1) = x_t + x_dot * dt
- y_(t+1) = y_t + y_dot * dt
- v_(t+1) = clamp(v_t + v_dot * dt, -v_max, +v_max)
- psi_(t+1) = psi_t + psi_dot * dt

> Limits (max steer, max accel, etc.) are held in `BicycleModelLimits`.

---

### Parking environment

`ParkingEnv` is designed as a Gym‑style environment (in progress):
- `reset()` : sample parking slot pose + reset car pose
- `step(action, simDt)` : apply an action, update state, compute reward, termination
- `reward()` : compute shaping / sparse reward (TBD)

### Parking pose randomization
The slot pose is randomized using `Randomizer`:
- `setParkingPos(minX,maxX,minY,maxY)` → `{x,y}`
- `setParkingYaw()` → yaw

### Parking success check (slot frame)
A robust check uses the slot coordinate frame:
1. transform car center into slot frame: `rel = worldToSlot(carPos, slotPos, slotYaw)`
2. compute heading error: `psiRel = wrapPi(carYaw - slotYaw)`
3. check tolerances: `|rel.x|`, `|rel.y|`, `|psiRel|`

### Global coordinate system to local coordinate system of the car
This section the coordinate of the parking space corner points is introduced. It is the transformed coordinate system from the global coordindate system to the local coordinate system. In global coordinate systems, the car must account for its own position and orientation within the global frame, complicating calculations. 
Expressing a global coordinate system as a local coordinate system simplifies the representation, making it easier to manage and understand. 
In the local coordinate system, the front side of the car is positive along the y-axis, and the right side of the car is position along the x-axie.


#### Frames (used in this transform)
| Frame | Description |
|---|---|
|**World frame** | +X right, +Y up. Angles (yaw) are **CCW-positive** |
|**Slot frame** | origin at the parking slot center. +x = slot “right”, +y = slot “forward” (aligned with `slotYaw`) |
|**Car observation frame** (for `Observation.distCorners`) | origin at the car center. **+x = car right**, **+y = car forward** |


#### Step-by-step transform (slot corners → world → car)
Let:
- `parkingPos = (xs, ys)`, `slotYaw = θs`
- `carPos  = (xc, yc)`, `carYaw  = ψ`
- `halfWid = PARKING_WIDTH / 2`, `halfLen = PARKING_LENGTH / 2`

**1) Define the 4 corners in the slot frame**
(ordered: front-right, front-left, rear-left, rear-right)

- `c0 = (+halfWid, +halfLen)`
- `c1 = (-halfWid, +halfLen)`
- `c2 = (-halfWid, -halfLen)`
- `c3 = (+halfWid, -halfLen)`

**2) Slot → World (rotate then translate)**
Using the standard CCW rotation matrix:

`R(θ) = [[cosθ, -sinθ], [sinθ, cosθ]]`

`cornerWorld = slotPos + R(slotYaw) * cornerSlot`

**3) World → Car observation frame**
First translate to car origin:

`d = cornerWorld - carPos = (dx, dy)`

Then project onto the car’s **right** and **forward** unit vectors:

- `forward = (cosψ, sinψ)`
- `right   = (sinψ, -cosψ)`   (forward rotated -90°)

So the car-local coordinates are:

- `x_local = dot(d, right)   = dx*sinψ - dy*cosψ`
- `y_local = dot(d, forward) = dx*cosψ + dy*sinψ`

These `(x_local, y_local)` values are stored in `Observation.distCorners[i]`.

#### Normalization (Will be added later for RL)
To keep observations bounded, normalize by a chosen `maxDist` and clamp:

- `x_norm = clamp(x_local / maxDist, -1, +1)`
- `y_norm = clamp(y_local / maxDist, -1, +1)`

`maxDist` should be a **task-stable** scale (e.g., half-diagonal of the parking region), not something that changes with window size.

---

## Critical OpenGL lifetime rules (hard-won lessons)

### Rule 1: Create the OpenGL context **before** creating GL resources
- `Window` should own: `glfwInit`, `glfwCreateWindow`, `gladLoadGLLoader`.
- Only after that may you construct:
  - `RectShader` / `ShaderProgram`
  - `Loader` (VAO/VBO/EBO)
  - any class that calls `gl*`

### Rule 2: Avoid copying RAII GL owners
If a class deletes a GL handle in its destructor (program/VAO/VBO/EBO),
then code like this is dangerous:

```cpp
rectShader = RectShader();  // temporary may delete program ID
quad      = Loader(...);    // temporary may delete VAO/VBO/EBO
```

Recommended solutions:
- Make these classes **move-only** (`delete` copy ctor/assign; implement move).
- Or store them as `std::unique_ptr` and construct once after GL is ready.

---


## CI process
- [CI process](docs/CI_Process.md)

## Troubleshooting
- [Troubleshooting](docs/Troubleshooting.md)
