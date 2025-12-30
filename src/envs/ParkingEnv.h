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
    std::array<Position2D, 4> distCorners;  // the relative coordinate system of the car from the parking lot corners to the center of the car
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
    ParkingEnv(Randomizer* randomizer);

    /** 
     * @brief Step the environment by one time step, given an action. 
     * This function advance the world by a fixed amount of simulated time, 
     * then return observation for now. (reward, done, info, etc will be added later).
     * 
     * @return Observation
     */
    Observation step(Action&action, const float& simDt);
    
    /**
     * @brief Reset the environment to an initial state and return observation.
     * 
     * @return Observation
     * 
    */
    void reset();

    /**
     * @brief Return reward based on parking-success check
     * 
     * @return float
     */
    float reward();

    // getter 
    Observation getObservation() const { return observation; }
    VehicleState getVehicleState() const { return vehicleState; }
    Position2D getParkingPos() const { return parkingPos; }
    float getParkingYaw() const { return parkingYaw; }
    
    
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
    Position2D parkingPos{0.0f, 0.0f};      // parking lot position
    float parkingYaw{0.0f};                 // parking lot yaw

    Randomizer* randomizer{nullptr};
    BicycleModel bicycleModel{CAR_LENGTH};

    // helper functions
    Position2D setParkingPos(float minX, float maxX, float minY, float maxY);
    float setParkingYaw();

    // transform car position into the parking lot frame
    Position2D worldToSlot(const Position2D& carPos, const Position2D& slotPos, float slotYaw);

    // parking check functions
    bool isParked(const Position2D& carPos, float carYaw, const Position2D& parkingPos, float parkingYaw);
    bool isParkedAtCenter(const Position2D& carPos, const float carYaw, const Position2D& parkingPos, const float& parkingYaw);
 
};
#endif
