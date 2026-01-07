#include <gtest/gtest.h>
#include <array>
#include <cmath>

#include "../envs/ParkingEnv.h"
#include "../envs/ParkingParams.h"
#include "../vehicledynamics/VehicleTypes.h"
#include "../utilities/Randomizer.h"
#include "../utilities/MathUtils.h"


// temporal function for temporary
int calAdd(int a, int b) {
    return a + b;
}

// tests
TEST(calAddTest, calAddTestExample) {

    std::cout << "Test is executed " << std::endl;
    EXPECT_EQ(calAdd(1, 2), 3);
    EXPECT_EQ(calAdd(-1, 2), 1);
    EXPECT_EQ(calAdd(0, 0), 0);
}

// Common helpers/constants for these tests (put near the top of test_parking_math.cpp)
namespace {
    constexpr float kEps = 1e-4f;

    void ExpectPosNear(const Position2D& a, const Position2D& b) {
        EXPECT_NEAR(a.x, b.x, kEps);
        EXPECT_NEAR(a.y, b.y, kEps);
    }
    
    static constexpr float kPi = 3.14159265358979323846f;
    static constexpr float kDeg90 = kPi * 0.5f;
}


// Test case 1:
// carPos = (10,10), carYaw = 0
// slotPos = (5,5), slotYaw = 0
// slot corners in world (given by you):
//   (6.75, 8), (6.75, 2), (3.25, 2), (3.25, 8)
// expected car-frame vectors (carYaw=0 => car frame == world frame):
//   (-3.25, -2), (-3.25, -8), (-6.75, -8), (-6.75, -2)
TEST(ParkingEnvObs, CarFrameVectors_CarYaw0_SlotYaw0) {
    Randomizer randomizer;
    ParkingEnv env(&randomizer);

    const Position2D carPos{10.0f, 10.0f};
    const float carYaw = 0.0f;

    const Position2D slotPos{5.0f, 5.0f};
    const float slotYaw = 0.0f;

    const auto got = env.getCalculateRelCorners(carPos, carYaw, slotPos, slotYaw);

    const std::array<Position2D, 4> expected = {
        Position2D{-3.25f, -2.0f},
        Position2D{-3.25f, -8.0f},
        Position2D{-6.75f, -8.0f},
        Position2D{-6.75f, -2.0f}
    };

    // If your calcObsCurrentState returns corners in a different order,
    // reorder expected[] to match your corner order.
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
    Randomizer randomizer;
    ParkingEnv env(&randomizer);

    const Position2D carPos{10.0f, 10.0f};
    const float carYaw = kDeg90;

    const Position2D slotPos{5.0f, 5.0f};
    const float slotYaw = 0.0f;

    const auto got = env.getCalculateRelCorners(carPos, carYaw, slotPos, slotYaw);

    const std::array<Position2D, 4> expected = {
        Position2D{-2.0f,  3.25f},
        Position2D{-8.0f,  3.25f},
        Position2D{-8.0f,  6.75f},
        Position2D{-2.0f,  6.75f}
    };

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
    Randomizer randomizer;
    ParkingEnv env(&randomizer);

    const Position2D carPos{10.0f, 10.0f};
    const float carYaw = 0.0f;

    const Position2D slotPos{5.0f, 5.0f};
    const float slotYaw = kDeg90;

    const auto got = env.getCalculateRelCorners(carPos, carYaw, slotPos, slotYaw);

    const std::array<Position2D, 4> expected = {
        Position2D{-8.0f, -3.25f},
        Position2D{-2.0f, -3.25f},
        Position2D{-2.0f, -6.75f},
        Position2D{-8.0f, -6.75f}
    };

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
    Randomizer randomizer;
    ParkingEnv env(&randomizer);

    const Position2D carPos{10.0f, 10.0f};
    const float carYaw = kDeg90;

    const Position2D slotPos{5.0f, 5.0f};
    const float slotYaw = kDeg90;

    const auto got = env.getCalculateRelCorners(carPos, carYaw, slotPos, slotYaw);

    const std::array<Position2D, 4> expected = {
        Position2D{-3.25f, 8.0f},
        Position2D{-3.25f, 2.0f},
        Position2D{-6.75f, 2.0f},
        Position2D{-6.75f, 8.0f}
    };

    for (int i = 0; i < 4; ++i) {
        ExpectPosNear(got[i], expected[i]);
    }
}
