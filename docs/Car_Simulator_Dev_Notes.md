# Car Simulator -- Dev Notes

## TODO
- Unify the usages of height and length.

## Overview
Top-down 2D car simulator: **car body + 4 wheels**, meters-first physics with a **fixed timestep**, and smooth rendering via a single **unit-quad** mesh and a **RectShader** (scale → rotate → translate).


## Simulator Environment

### Environment settings (source of truth)

| Element      | Description | Code |
|-----------|---------| --------- |
| Screen    | 800 pixel width × 600 pixel height | `SCR_WIDTH=800`, `SCR_HEIGHT=600`|
| Scale | 1 pixel is defined 0.05 m (5 cm). | `PPM = 20`  |
| Car size (m) |  2 m width × 4 m height  | `CAR_WIDTH=2.0f`, `CAR_HEIGHT=4.0f` |
| Parking size (m) | 4 m width × 8 m height | `PARKING_WIDTH=4.0f`, `PARKING_HEIGHT=8.0f` |
| Wheel geometry (m) | 0.35 m width × 0.75 m height  | `wheel.width=0.35f`, `wheel.length=0.75f`  |
| Simulation step | 0.01s  | `simDt=0.01` |
| Kinematic bicycle limits | acceleration <= 1.0 m/s², steering angle <= 45 degrees,  velocity <= 10km/h ≈ 2.78 m/s  | `a_max=1.0f`, `delta_max=0.785f`, `v_max=2.78f` |

### Units
In this project, **meters, radians and seconds** are used in physics simulations and NDC is used for rendering. 

### Coordinate frames & signs
- **World**: +X right, +Y up.
- **Vehicle local**: x = forward (longitudinal), y = left (lateral).
- **Angles**: yaw (psi) ψ is CCW-positive; steering angle δ (delta) is CCW-positive (left = +δ).

### Rendering
#### Mesh (Loader class)
Single **unit quad** centered at (0,0) with vertices at ±0.5; shared VAO/VBO/EBO.

#### Material (RectShader class) 
Uniforms: `uOffset` (NDC center), `uScale` (full NDC size), `uYaw` (CCW), `uColor`.

##### Vertex shader core rules
In `rectShader.vert`, **scale → rotate (CCW) → translate** is applied and **the offset** is not rotated.

#### Entity (Entity class)
It holds **pos_m** (x, y in m), **yaw** (rad), **size** (height, width in m), **color**, pointers to shared **Mesh** and **Material**.

#### Renderer pipeline (2D)
For each entity (car body, wheels, parking rectangle), rendering is executed in Renderer class with Loader, RectShader and Entity classes.
1. Convert meters → NDC with `metersToNDC(x, y)` and `rectSizeToNDC(width, height)`.
2. Set `uOffset`, `uYaw`, `uScale` and `uColor` by accessing RectShader pointer via Entity class.
3. Bind the shared VAO, draw with `glDrawElements`.

### Timing model (fixed sim, interpolated render)
- Accumulate `frameDt` into `accumulator`; simulate in chunks of `simDt = 0.01`.
- Clamp accumulator (e.g., ≤ `5*simDt`) to avoid spiral-of-death.
- Compute `alpha = accumulator / simDt` and **interpolate** once per frame:
  - `posDraw = lerp(prevPos, curPos, alpha)`
  - `yawDraw = lerpAngle(prevPsi, curPsi, alpha)`
  - `deltaDraw = lerp(prevDelta, curDelta, alpha)`
- Render **everything** (car + all wheels) from the same draw pose (`posDraw`, `yawDraw`, `deltaDraw`).

#### Input Actions
Discrete inputs: **acceleration** and **steering angle**.

### Vehicle Dynamics

#### Vehicle geometry & anchors
Car body sizes are 2 m width × 4 m height, and wheel sizes are 0.35 meter width × 0.75 meter height. 
Anchors in car frame (x forward, y left):
- FL ( +Lf, +track/2 ), FR ( +Lf, −track/2 )
- RL ( −Lr, +track/2 ), RR ( −Lr, −track/2 )

Keep wheels visually inside the body (choose margins):  
carLen = CAR_HEIGHT, carWid = CAR_WIDTH
carLen=4.0f;  
front_margin = 0.25 m, rear_margin = 0.25 m, side_margin = 0.10 m  
Lf = (carLen * 0.5f) - (wheel.length * 0.5f +front_margin);  
Lr = (carLen * 0.5f) - (wheel.length * 0.5f +rear_margin);  
track = carWid - (wheel.width + 2.0f *side_margin);


#### Kinematic Bicycle Model
Inputs: a (acceleration) and delta (steering angle)
Outputs: x, y, psi (heading angle), delta

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

