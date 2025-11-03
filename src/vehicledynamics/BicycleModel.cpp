#include <cmath>
#include <algorithm>
#include <iostream>
#include "BicycleModel.h"


// constructor
// ------------------------------------------------------------------------
BicycleModel::BicycleModel(float length): length(length) {};

// Calculate the car movement using kinematic bicycle model
// ------------------------------------------------------------------------
void BicycleModel::kinematicAct(Action& action, VehicleState& vehicleState, float dt) {
    
    const BicycleModelLimits limits;

    // clamp action inputs
    action.steeringAngle = std::clamp(action.steeringAngle, -limits.delta_max, limits.delta_max);
    action.acceleration = std::clamp(action.acceleration, -limits.a_max, limits.a_max);

    // update velocity with acceleration limits
    vehicleState.velocity += action.acceleration * dt;

    // clamp velocity
    vehicleState.velocity = std::clamp(vehicleState.velocity, -limits.v_max, limits.v_max);

    // compute derivatives
    const float x_dot = vehicleState.velocity * cosf(vehicleState.psi);
    const float y_dot = vehicleState.velocity * sinf(vehicleState.psi);
    const float v_dot = action.acceleration;
    const float psi_dot = vehicleState.velocity * tanf(action.steeringAngle) / length;

    vehicleState.delta = action.steeringAngle;

    // update state
    vehicleState.pos.x += dt * x_dot;
    vehicleState.pos.y += dt * y_dot;
    vehicleState.psi += dt * psi_dot;

    // update state
    // updateState(vehicleState, x_dot, y_dot, v_dot, psi_dot, dt);
    std::cout << "Kinematic Act: x: "<< vehicleState.pos.x << " y: " << vehicleState.pos.y << " psi:" << vehicleState.psi << " v: " << vehicleState.velocity << std::endl;
}

void BicycleModel::dynamicAct(Action action) {
    // To be implemented later

}

// upadte the state of the car
// ------------------------------------------------------------------------
void BicycleModel::updateState(VehicleState& vehicleState, const float& x_dot, const float& y_dot, const float& v_dot, const float& psi_dot, float& dt) {
    const BicycleModelLimits limits;
    
    // update state
    vehicleState.pos.x += dt * x_dot;
    vehicleState.pos.y += dt * y_dot;
    // vehicleState.velocity += dt * v_dot;
    vehicleState.psi += dt * psi_dot;

    // clip limits
    vehicleState.velocity = std::clamp(vehicleState.velocity + (dt * v_dot), -limits.v_max, limits.v_max);

}
