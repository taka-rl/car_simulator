#include <vector>
#include <iostream>
#include <math.h>
#include <fstream>
#include <thread>

using namespace std;


class Car{
public:
    // Constructors
    Car(float f, float r);

    // Methods
    void setConstants(float f, float r);
    void setInitialConditions(float x_i, float y_i, float v_i, float psi_i);
    void setInputs(float a, float steer);
    void simulateKinematics(float t);
    void simulateDynamics(float t);
    void writeToFile(string fileName);
    vector<float> positionX;
    vector<float> positionY;
    vector<float> velocity;
    vector<float> heading;
    vector<float> time;

private:
    const float dT;
    float frontDistance;
    float rearDistance;
    float slip;
    float acceleration_Input;
    float steerAngle_Input;
};
