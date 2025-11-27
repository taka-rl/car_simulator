#ifndef PARKINGPARAMS_H
#define PARKINGPARAMS_H


#include "../utilities/MathUtils.h"

// Parking success tolerances (in slot frame)
constexpr float PARK_LONG_TOL = 1.5f;   // ±1.5 m along slot axis on X axis
constexpr float PARK_LAT_TOL  = 1.f;    // ±1.0 m sideways on Y axis
constexpr float PARK_YAW_TOL  = 10.0f * (PI / 180.0f); // 10 deg in rad



#endif