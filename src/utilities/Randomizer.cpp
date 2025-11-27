#include "Randomizer.h"

// constructor
// ------------------------------------------------------------------------
Randomizer::Randomizer() {};

// return a random float number in [minVal, maxVal)
// ------------------------------------------------------------------------
float Randomizer::randFloat(float minVal, float maxVal) {
    std::uniform_real_distribution<float> dist(minVal, maxVal);
    return dist(g_rng);
}

// return int in [minVal, maxVal)
// ------------------------------------------------------------------------
int Randomizer::randInt(int minVal, int maxVal) {
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return dist(g_rng);
}
