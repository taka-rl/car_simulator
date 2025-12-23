#ifndef PARKINGENV_H
#define PARKINGENV_H

#include <iostream>
#include <string>
#include <array>

#include "ParkingParams.h"
#include "../core/Config.h"
#include "../utilities/Randomizer.h" 
#include "../vehicledynamics/VehicleTypes.h"
#include "../vehicledynamics/BicycleModel.h"


// forward declarations at global scope
class Simulator;
class Randomizer;
class BicycleModel;
struct VehicleState;
struct Action;


// observation structure
struct Observation {
    std::array<std::array<State, 1>, 4> distCorners;  // the relative coordinate system of the car from the parking lot corners to the center of the car
    VehicleState vehicleState;
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
    ParkingEnv(Randomizer* randomizer, BicycleModel* bicycleModel);

    /** 
     * Step the environment by one time step, given an action, advance the world
     * by a fixed amount of simulated time, then return observation, reward, done, info.
     * --------------------------------------------------------------------
     * @return Observation
     */
    Observation step(Action&action, const float& simDt);
    
    /**
     * Reset the environment to an initial state and return observation.
     * --------------------------------------------------------------------
     * @return Observation
     * 
    */
    void reset();
    float reward();

    // getter 
    Observation getObservation() const { return observation; }
    VehicleState getVehicleState() const { return vehicleState; }
    State getParkingPos() const { return parkingPos; }
    float getParkingYaw() const { return parkingYaw; }
    

    State setParkingPos(float minX, float maxX, float minY, float maxY);
    float setParkingYaw();
    bool isParked(const State& carPos, float carYaw, const State& parkingPos, float parkingYaw);

    
private:
    // RL attributes
    std::string actionType;                 // discrete or continuous
    std::array<float, 2> actionSpace;       // the dimention of the action
    std::array<float, 2> observationSpace;  // state information

    Observation observation{};              // current observation
    float rewardValue{0.0f};                // reward value

    // car attributes
    VehicleState vehicleState{};            // current vehicle state

    // Parking lot attributes
    State parkingPos{0.0f, 0.0f};          // parking lot position
    float parkingYaw{0.0f};                 // parking lot yaw

    Randomizer* randomizer{nullptr};
    BicycleModel* bicycleModel{nullptr};

    State worldToSlot(const State& carPos, const State& slotPos, float slotYaw);
    bool isParkedAtCenter(const State& carPos, const float carYaw, const State& parkingPos, const float& parkingYaw);
 
};
#endif
