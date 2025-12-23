#include "ParkingEnv.h"

// constructor
ParkingEnv::ParkingEnv(Randomizer* randomizer, BicycleModel* bicycleModel) : randomizer(randomizer), bicycleModel(bicycleModel){};


Observation ParkingEnv::step(Action&action, const float& simDt) {

    // apply the action using bicycle model
    bicycleModel->kinematicAct(action, vehicleState, simDt);

    // reward calculation
    rewardValue = reward();

    return Observation { 
        {
            Position2D{parkingPos.x + 5.0f - vehicleState.pos.x, parkingPos.y + 5.0f - vehicleState.pos.y}, 
            Position2D{parkingPos.x - 5.0f - vehicleState.pos.x, parkingPos.y + 5.0f - vehicleState.pos.y}, 
            Position2D{parkingPos.x - 5.0f - vehicleState.pos.x, parkingPos.y - 5.0f - vehicleState.pos.y}, 
            Position2D{parkingPos.x + 5.0f - vehicleState.pos.x, parkingPos.y - 5.0f - vehicleState.pos.y}
        },
        vehicleState
    };
    
}
// reset the environement to initial state
// ------------------------------------------------------------------------
void ParkingEnv::reset() {
    // random positions and yaw for parking
    const Position2D randParkingPos = setParkingPos(-15.f, 15.f, -10.f, 10.f);  // temporary values
    parkingPos = randParkingPos;
    parkingYaw = setParkingYaw();

    // random positions and yaw for car
    const float marginX = randomizer->randFloat(-5.0, 5.0), marginY = randomizer->randFloat(-5.0, 5.0);
    const Position2D randCarPos = {randParkingPos.x + marginX, randParkingPos.y + marginY};

    // calculate each corner of the parking lot relative to the car position
    const Position2D relCorner1 = worldToSlot({randParkingPos.x + 5.0f, randParkingPos.y + 5.0f}, randParkingPos, parkingYaw);
    const Position2D relCorner2 = worldToSlot({randParkingPos.x - 5.0f, randParkingPos.y + 5.0f}, randParkingPos, parkingYaw);
    const Position2D relCorner3 = worldToSlot({randParkingPos.x - 5.0f, randParkingPos.y - 5.0f}, randParkingPos, parkingYaw);
    const Position2D relCorner4 = worldToSlot({randParkingPos.x + 5.0f, randParkingPos.y - 5.0f}, randParkingPos, parkingYaw);

    // set observation
    vehicleState.pos = randCarPos;
    vehicleState.psi = 0.0f;
    vehicleState.velocity = 0.0f;
    vehicleState.delta = 0.0f;

    observation.vehicleState = vehicleState;
    observation.distCorners = {
        Position2D{relCorner1.x - randCarPos.x, relCorner1.y - randCarPos.y}, Position2D{relCorner2.x - randCarPos.x, relCorner2.y - randCarPos.y}, 
        Position2D{relCorner3.x - randCarPos.x, relCorner3.y - randCarPos.y}, Position2D{relCorner4.x - randCarPos.x, relCorner4.y - randCarPos.y}
    };

}

float ParkingEnv::reward() {
    // check parking success
    bool parkingSuccess = isParked({vehicleState.pos.x, vehicleState.pos.y}, vehicleState.psi, {parkingPos.x, parkingPos.y}, parkingYaw);
    if (parkingSuccess) {
        return 1.0f;
    } else {
        return 0.0f;
    }
}

// set the parking lot location randomly
// ------------------------------------------------------------------------
Position2D ParkingEnv::setParkingPos(float minX, float maxX, float minY, float maxY) {
    float x = randomizer->randFloat(minX, maxX);
    float y = randomizer->randFloat(minY, maxY);
    return {x, y};
};

// return yaw either 0 or 90 degree
// ------------------------------------------------------------------------
float ParkingEnv::setParkingYaw() {
    const int k = randomizer->randInt(0, 1);  // Currently yaw degree shall be 0 or 90 degree
    float yawDeg = (k == 0) ? 0.0f : 90.0f;
    return yawDeg * PI / 180.0f;
}

// rotate car poistion into the parking lot frame
// ------------------------------------------------------------------------
Position2D ParkingEnv::worldToSlot(const Position2D& carPos, const Position2D& slotPos, float slotYaw) {
    const float dx = carPos.x - slotPos.x;
    const float dy = carPos.y - slotPos.y;

    const float c = cosf(slotYaw);
    const float s = sinf(slotYaw);

    return Position2D{
        c * dx + s * dy,
        -s * dx + c * dy
    };
}

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
bool ParkingEnv::isParkedAtCenter(const Position2D& carPos, const float carYaw, const Position2D& parkingPos, const float& parkingYaw) {

    // car center in slot frame
    Position2D rel = worldToSlot(carPos, parkingPos, parkingYaw);

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
bool ParkingEnv::isParked(const Position2D& carPos, float carYaw, const Position2D& parkingPos, float parkingYaw) {

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
    Position2D rel;
    rel.x =  cSlot * dx + sSlot * dy;  // along slot length
    rel.y = -sSlot * dx + cSlot * dy;  // along slot width

    // Car orientation relative to slot
    const float psiRel = wrapPi(carYaw - parkingYaw);
    const float cRel   = std::cos(psiRel);
    const float sRel   = std::sin(psiRel);

    // Car corners in *car* local frame (x forward, y left)
    // Four corners: (±halfLen, ±halfWid)
    std::array<Position2D, 4> carLocalCorners = {{
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
