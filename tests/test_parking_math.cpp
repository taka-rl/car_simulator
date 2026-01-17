#include <gtest/gtest.h>
#include <array>
#include <cmath>

#include "envs/ParkingEnv.h"
#include "envs/ParkingParams.h"
#include "vehicledynamics/VehicleTypes.h"
#include "utilities/Randomizer.h"
#include "utilities/MathUtils.h"


// Common helpers/constants for these tests (put near the top of test_parking_math.cpp)
namespace {
    // Tolerance for floating point computations
    constexpr float kEps = 1e-4f;

    // Helper function to compare positions
    void ExpectPosNear(const Position2D& a, const Position2D& b) {
        EXPECT_NEAR(a.x, b.x, kEps);
        EXPECT_NEAR(a.y, b.y, kEps);
    }
    
    static constexpr float kPi = 3.14159265358979323846f;  // pi constant same as in MathUtils.h
    static constexpr float kDeg90 = kPi * 0.5f;
    
    // common functions/constants for these tests
    // Setup ParkingEnv for tests
    ParkingEnv setUpEnv() {
        Randomizer randomizer;
        ParkingEnv env(&randomizer);
        return env;
    }
}


// Test cases for ParkingEnv::calculateRelCorners
// -------------------------------------------------------------------------------------------------

// Test case 1:
// carPos = (10,10), carYaw = 0
// slotPos = (5,5), slotYaw = 0
// slot corners in world (given by you):
//   (6.75, 8), (6.75, 2), (3.25, 2), (3.25, 8)
// expected car-frame vectors (carYaw=0 => car frame == world frame):
//   (-3.25, -2), (-3.25, -8), (-6.75, -8), (-6.75, -2)
TEST(ParkingEnvObs, CarFrameVectors_CarYaw0_SlotYaw0) {
    // Setup environment
    ParkingEnv env = setUpEnv();

    // Define test inputs
    const Position2D carPos{10.0f, 10.0f};
    const float carYaw = 0.0f;

    const Position2D slotPos{5.0f, 5.0f};
    const float slotYaw = 0.0f;

    // Call the function under test
    const auto got = env.getCalculateRelCorners(carPos, carYaw, slotPos, slotYaw);

    // Define expected outputs
    const std::array<Position2D, 4> expected = {
        Position2D{-3.25f, -2.0f},
        Position2D{-3.25f, -8.0f},
        Position2D{-6.75f, -8.0f},
        Position2D{-6.75f, -2.0f}
    };

    // Verify results
    for (int i = 0; i < 4; ++i) {
        ExpectPosNear(got[i], expected[i]);
    }
}

// Test case 2:
// carPos = (10,10), carYaw = 90deg
// slotPos = (5,5),  slotYaw = 0
// slot corners in world:
//   (6.75, 8), (6.75, 2), (3.25, 2), (3.25, 8)
// expected car-frame vectors (rotate by R(-90deg): (dx,dy)->(dy,-dx)):
//   (-2, 3.25), (-8, 3.25), (-8, 6.75), (-2, 6.75)
TEST(ParkingEnvObs, CarFrameVectors_CarYaw90_SlotYaw0) {
    // Setup environment
    ParkingEnv env = setUpEnv();

    // Define test inputs
    const Position2D carPos{10.0f, 10.0f};
    const float carYaw = kDeg90;

    const Position2D slotPos{5.0f, 5.0f};
    const float slotYaw = 0.0f;

    // Call the function under test
    const auto got = env.getCalculateRelCorners(carPos, carYaw, slotPos, slotYaw);

    // Define expected outputs
    const std::array<Position2D, 4> expected = {
        Position2D{-2.0f,  3.25f},
        Position2D{-8.0f,  3.25f},
        Position2D{-8.0f,  6.75f},
        Position2D{-2.0f,  6.75f}
    };

    // Verify results
    for (int i = 0; i < 4; ++i) {
        ExpectPosNear(got[i], expected[i]);
    }
}

// Test case 3:
// carPos = (10,10), carYaw = 0
// slotPos = (5,5),  slotYaw = 90deg
// slot corners in world:
//   (2, 6.75), (8, 6.75), (8, 3.25), (2, 3.25)
// expected car-frame vectors (carYaw=0 => car frame == world frame):
//   (-8, -3.25), (-2, -3.25), (-2, -6.75), (-8, -6.75)
TEST(ParkingEnvObs, CarFrameVectors_CarYaw0_SlotYaw90) {
    // Setup environment
    ParkingEnv env = setUpEnv();
    
    // Define test inputs
    const Position2D carPos{10.0f, 10.0f};
    const float carYaw = 0.0f;

    const Position2D slotPos{5.0f, 5.0f};
    const float slotYaw = kDeg90;

    // Call the function under test
    const auto got = env.getCalculateRelCorners(carPos, carYaw, slotPos, slotYaw);

    // Define expected outputs
    const std::array<Position2D, 4> expected = {
        Position2D{-8.0f, -3.25f},
        Position2D{-2.0f, -3.25f},
        Position2D{-2.0f, -6.75f},
        Position2D{-8.0f, -6.75f}
    };

    // Verify results
    for (int i = 0; i < 4; ++i) {
        ExpectPosNear(got[i], expected[i]);
    }
}

// Test case 4:
// carPos = (10,10), carYaw = 90deg
// slotPos = (5,5),  slotYaw = 90deg
// slot corners in world:
//   (2, 6.75), (8, 6.75), (8, 3.25), (2, 3.25)
// expected car-frame vectors (rotate by R(-90deg): (dx,dy)->(dy,-dx)):
//   (-3.25, 8), (-3.25, 2), (-6.75, 2), (-6.75, 8)
TEST(ParkingEnvObs, CarFrameVectors_CarYaw90_SlotYaw90) {
    // Setup environment
    ParkingEnv env = setUpEnv();

    // Define test inputs
    const Position2D carPos{10.0f, 10.0f};
    const float carYaw = kDeg90;

    const Position2D slotPos{5.0f, 5.0f};
    const float slotYaw = kDeg90;

    // Call the function under test
    const auto got = env.getCalculateRelCorners(carPos, carYaw, slotPos, slotYaw);

    // Define expected outputs
    const std::array<Position2D, 4> expected = {
        Position2D{-3.25f, 8.0f},
        Position2D{-3.25f, 2.0f},
        Position2D{-6.75f, 2.0f},
        Position2D{-6.75f, 8.0f}
    };

    // Verify results
    for (int i = 0; i < 4; ++i) {
        ExpectPosNear(got[i], expected[i]);
    }
}
