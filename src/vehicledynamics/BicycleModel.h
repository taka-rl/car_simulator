#ifndef BICYCLEMODEL_H
#define BICYCLEMODEL_H


#include <cmath>
#include <algorithm>
#include <iostream>

#include "../envs/ParkingEnv.h"
#include "../utilities/MathUtils.h"


struct BicycleModelLimits {
    float delta_max{PI * 0.25f};     // PI/4 = 45 degrees = 0.785rad
    float delta_rate_max{0.6f};      // rad/s  
    float a_max{1.0f};               // 1.0 m/s^2
    float v_max{2.78f};              // 10 km/h ≈ 2.78 m/s
};

/**
 * Bicycle Model Class
 * ---------------------------
 * This class implements the kinematic bicycle model for vehicle dynamics. 
 * Dynamic bicycle model will be implemeted later.
 */
class BicycleModel {

public:
    BicycleModel(float length);

    /** Calculate the car movement using kinematic bicycle model
     * ----------------------------------------------------------------------------
     * @param[in] action: Action structure containing acceleration and steering angle
     * @param[in] s: State structure to be updated
     * @param[in] dt: discrete time step
     * Kinematic bicycle model equations:
     * x_dot = v * cos(psi);
     * y_dot = v * sin(psi);
     * v_dot = acceleration;
     * psi_dot = v * tan(steeringAngle) / Length;
     * @return void 
    */
    void kinematicAct(Action& action, VehicleState& vehicleState, float dt);

    // Calculate the car movement using dynamic bicycle model
    void dynamicAct(Action action);

    /** Update the state of the car
     * @param[in] vehicleState: VehicleState structure to be updated
     * @param[in] x_dot: derivative of x position
     * @param[in] y_dot: derivative of y position
     * @param[in] v_dot: derivative of velocity
     * @param[in] psi_dot: derivative of heading angle
     * @param[in] dt: discrete time step
     * @return void
     */
    void updateState(VehicleState& vehicleState, const float& x_dot, const float& y_dot, const float& v_dot, const float& psi_dot, float& dt);

private:
    float length{0.0f};

};
#endif
