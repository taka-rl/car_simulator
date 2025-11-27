#ifndef PARKINGENV_H
#define PARKINGENV_H

#include <iostream>
#include <string>
#include <array>

#include "ParkingParams.h"
#include "../core/Config.h"
#include "../utilities/Randomizer.h" 


// forward declarations at global scope
class Simulator;
class Randomizer;


// consider definition location for the following structs later
struct State { float x, y; };

struct Action {
    float acceleration{0.0f};
    float steeringAngle{0.0f};
};

struct VehicleState {
    State pos{0.0f, 0.0f};
    float psi{0.0f};
    float velocity{0.0f};
    float delta{0.0f};
};

/**
 * Parking Env Class
 * ---------------------------
 * This class, which is Gymnasium-style environment, contains init, step, reward and reset functions for simulations. 
 */
class ParkingEnv {
public:
    // constructor
    // ------------------------------------------------------------------------
    ParkingEnv(Randomizer* randomizer);

    void step();
    void reset();
    void reward();

    State setParkingPos(float minX, float maxX, float minY, float maxY);
    float setParkingYaw();
    bool isParked(const State& carPos, float carYaw, const State& parkingPos, float parkingYaw);

    
private:
    std::string actionType;                 // discrete or continuous
    std::array<float, 2> actionSpace;       // the dimention of the action
    std::array<float, 2> observationSpace;  // state information
    Randomizer* randomizer{nullptr};

    float calcReward();
    void getState();


    State worldToSlot(const State& carPos, const State& slotPos, float slotYaw);
    bool isParkedAtCenter(const State& carPos, const float carYaw, const State& parkingPos, const float& parkingYaw);
 
};
#endif
