# Class Diagram

## Inputs

This diagram was generated from these headers:

- `BicycleModel.h`
- `Config.h`
- `Entity.h`
- `Loader.h`
- `MathUtils.h`
- `ParkingEnv.h`
- `ParkingParams.h`
- `Randomizer.h`
- `RectShader.h`
- `Renderer.h`
- `ShaderProgram.h`
- `Simulator.h`
- `VehicleTypes.h`
- `Window.h`

## Mermaid UML

```mermaid
classDiagram

class Action {
  <<struct>>
  +acceleration : float
  +steeringAngle : float
}

class BicycleModel {
  -length : float
  +BicycleModel(float length)
  +dynamicAct(Action action) : void
  +kinematicAct(Action& action, VehicleState& vehicleState, float dt) : void
  +updateState(VehicleState& vehicleState, const float& x_dot, const float& y_dot, const float& v_dot, const float& psi_dot, float& dt) : void
}

class BicycleModelLimits {
  <<struct>>
  +a_max : float
  +delta_max : float
  +delta_rate_max : float
  +v_max : float
}

class Entity {
  +color : std::array<float, 4>
  +length_m : float
  +loader : Loader*
  +pos_m : State
  +rectShader : RectShader*
  +width_m : float
  +yaw : float
  +Entity(Loader* loader, RectShader* rectShader)
  +getColor() : const std::array<float, 4>& const noexcept
  +getLength() : float const noexcept
  +getPosX() : float const noexcept
  +getPosY() : float const noexcept
  +getWidth() : float const noexcept
  +getYaw() : float const noexcept
  +setColor(const std::array<float, 4>& newColor) : void
  +setLength(const float newLength) : void
  +setPos(const State& newPos) : void
  +setWidth(const float newWidth) : void
  +setYaw(const float newYaw) : void
}

class Loader {
  -ebo : unsigned int
  -vao : unsigned int
  -vbo : unsigned int
  +Loader(float vertices[], size_t vertexCount, unsigned int indices[], size_t indexCount)
  +getEBO() : unsigned int const
  +getVAO() : unsigned int const
  +getVBO() : unsigned int const
  +~Loader()
  -makeEBO() : unsigned int
  -makeVAO() : unsigned int
  -makeVBO() : unsigned int
}

class ParkingEnv {
  -actionSpace : std::array<float, 2>
  -actionType : std::string
  -observationSpace : std::array<float, 2>
  -randomizer : Randomizer*
  +ParkingEnv(Randomizer* randomizer)
  +isParked(const State& carPos, float carYaw, const State& parkingPos, float parkingYaw) : bool
  +reset() : void
  +reward() : void
  +setParkingPos(float minX, float maxX, float minY, float maxY) : State
  +setParkingYaw() : float
  +step() : void
  -calcReward() : float
  -getState() : void
  -isParkedAtCenter(const State& carPos, const float carYaw, const State& parkingPos, const float& parkingYaw) : bool
  -worldToSlot(const State& carPos, const State& slotPos, float slotYaw) : State
}

class Randomizer {
  -g_rng : std::mt19937
  +Randomizer()
  +randFloat(float minVal, float maxVal) : float
  +randInt(int minVal, int maxVal) : int
}

class RectShader {
  -uColorLoc_ : int
  -uOffsetLoc_ : int
  -uScaleLoc_ : int
  -uYawLoc_ : int
  +RectShader()
  +getUColor() : const int const noexcept
  +getUOffsetLoc() : const int const noexcept
  +setColor(float r, float g, float b, float a) : void const
  +setOffset(float x, float y) : void const
  +setScale(float x, float y) : void const
  +setYaw(float yaw) : void const
  -cacheColorLocation() : void
  -cacheOffsetLocation() : void
  -cacheScaleLocation() : void
  -cacheYawLocation() : void
}

class Renderer {
  -fbH : int
  -fbW : int
  -ppm : float
  +Renderer(float ppm, int fbW, int fbH)
  +draw(const Entity& e) : void const
  -metersToNDC(float x_m, float y_m) : State const
  -rectSizeToNDC(float width_m, float length_m) : State const
}

class ShaderPaths {
  <<struct>>
  +fragmentPath : const std::string
  +vertexPath : const std::string
}

class ShaderProgram {
  #ID : unsigned int
  +ShaderProgram(ShaderPaths paths)
  +getShaderID() : const unsigned int const noexcept
  +setBool(const std::string &name, bool value) : void const
  +setFloat(const std::string &name, float value) : void const
  +setInt(const std::string &name, int value) : void const
  +use() : void const
  +~ShaderProgram()
  -checkCompileErrors(unsigned int shader, const std::string type) : void
  -loadShaderSource(unsigned int shader, std::string path) : int
  -makeShader(ShaderPaths paths) : int
}

class Simulator {
  -accumulator : double
  -action : Action
  -anchors : std::array<std::array<float, 2>, 4>
  -bicycleModel : BicycleModel
  -carEntity : Entity
  -curDelta : float
  -curPsi : float
  -curState : State
  -env : ParkingEnv
  -fbH : int
  -fbW : int
  -lastTime : double
  -parkingEntity : Entity
  -prevDelta : float
  -prevPsi : float
  -prevState : State
  -quad : Loader
  -randomizer : Randomizer
  -rectShader : RectShader
  -renderer : Renderer
  -simDt : const double
  -trajectoryEntities : std::vector<Entity>
  -vehicleParams : VehicleParams
  -vehicleState : VehicleState
  -wheelFL : Entity
  -wheelFR : Entity
  -wheelRL : Entity
  -wheelRR : Entity
  -window : GLFWwindow*
  +Simulator()
  +init() : bool
  +run() : void
  +tick() : void
  -clampAccumulator(double& accum, const double simDt, double maxSteps = 5.0) : void
  -initEntities() : void
  -initRenderer() : void
  -initSimulationState() : void
  -initWindowAndGL() : bool
  -interp(const State& prev, const State& curr, float alpha) : State
  -keepOnScreenMeters(State& s, float width_m, float length_m, int fbW, int fbH, float ppm) : void
  -lerp(float a, float b, float t) : float
  -placeWheel(Entity& wheel, float ax, float ay, bool front, const State& pos, const float& yawDraw, const float& steer) : void
}

class State {
  <<struct>>
  +x : float
  +y : float
}

class VehicleParams {
  <<struct>>
  +Lf : float
  +Lr : float
  +carLen : float
  +carWid : float
  +front_margin : float
  +rear_margin : float
  +side_margin : float
  +track : float
  +wheel : WheelSize
  +finalize() : void
}

class VehicleState {
  <<struct>>
  +delta : float
  +pos : State
  +psi : float
  +velocity : float
}

class WheelSize {
  <<struct>>
  +length : float
  +width : float
}

class Window {
  -m_window : GLFWwindow*
  -s_glfwInitialized : static bool
  +Window(int width, int height, const char* title)
  +get() : GLFWwindow* const
  +isValid() : bool const
  +~Window()
}

%% Relationships (inferred from inheritance + member fields)
RectShader --|> ShaderProgram
Entity *-- State : pos_m
Entity --> Loader : loader
Entity --> RectShader : rectShader
ParkingEnv --> Randomizer : randomizer
Simulator *-- VehicleState : vehicleState
Simulator *-- VehicleParams : vehicleParams
Simulator *-- BicycleModel : bicycleModel
Simulator *-- Randomizer : randomizer
Simulator *-- ParkingEnv : env
Simulator *-- Action : action
Simulator *-- RectShader : rectShader
Simulator *-- Loader : quad
Simulator *-- Renderer : renderer
Simulator *-- Entity : carEntity
Simulator *-- Entity : parkingEntity
Simulator *-- Entity : wheelFL
Simulator *-- Entity : wheelFR
Simulator *-- Entity : wheelRL
Simulator *-- Entity : wheelRR
Simulator *-- Entity : trajectoryEntities
Simulator *-- State : prevState
Simulator *-- State : curState
VehicleState *-- State : pos
VehicleParams *-- WheelSize : wheel
```

## Notes

- `*--` indicates ownership/composition (member-by-value or smart-pointer ownership).
- `-->` indicates a non-owning pointer/reference association.
- Relationships are inferred automatically from member field types; if something looks off, adjust the type or add a manual note.