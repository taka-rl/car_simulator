#ifndef RANDOMIZER_H
#define RANDOMIZER_H

#include <random>

/** Ranomizer class
 * ---------------------------
 * 
*/
class Randomizer {
public:

    // constructor
    // ------------------------------------------------------------------------
    Randomizer();

    /** Return a random float number
     * ----------------------------------------------------------------------------
     * @param[in] minVal: Minimum value
     * @param[in] maxVal: Maximum value
     * @return float 
     */
    float randFloat(float minVal, float maxVal);

    /** Return a random int number
     * ----------------------------------------------------------------------------
     * @param[in] minVal: Minimum value
     * @param[in] maxVal: Maximum value
     * @return int
     */
    int randInt(int minVal, int maxVal);

private:
    // RNG – constructed once
    std::mt19937 g_rng{ std::random_device{}() };
};
#endif