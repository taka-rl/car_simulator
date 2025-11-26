#ifndef VEHICLETYPES_H
#define VEHICLETYPES_H

#include "../core/Config.h"


// wheels
struct WheelSize { float length{0.75f}, width{0.35f}; };
struct VehicleParams {
    // car body
    float carWid{CAR_WIDTH}, carLen{CAR_LENGTH};

    // wheel
    WheelSize wheel{0.75f, 0.35f};

    // margins
    float front_margin{0.20f}, rear_margin{0.20f}, side_margin{0.10f};

    // wheel placement
    float Lf{};     // front wheel
    float Lr{};     // rear wheel
    float track{};  // wheel centers

    void finalize() {
        Lf = (carLen * 0.5f) - (wheel.length * 0.5f + front_margin);
        Lr = (carLen * 0.5f) - (wheel.length * 0.5f + rear_margin);
        track = carWid - (wheel.width + 2.0f * side_margin);
    }
};
#endif