#include "car.h"


Car::Car(float f, float r): dT(0.1) {
    setConstants(f, r);
}

void Car::setConstants(float f, float r) {
    frontDistance = f;
    rearDistance = r;
}

void Car::setInitialConditions(float x_i, float y_i, float v_i, float psi_i) {
    positionX.push_back(x_i);
    positionY.push_back(y_i);
    velocity.push_back(v_i);
    heading.push_back(psi_i);
    time.push_back(0.0);
}

void Car::setInputs(float a, float steer) {
    acceleration_Input = a;
    steerAngle_Input = steer;
}

void Car::simulateKinematics(float t) {
    /*
    Parameters:
        x, y: location
        a: acceleration
        δ(delta): steering angle
        v : velocity
        psi(ψ): the heading angle of the car
        β: slip angle
        L: Car length (L = frontDistance + rearDistance)

    Kinematics bicycle model
    Equation1:
        x_dot = v * cos(psi)
        y_dot = v * sin(psi)
        v_dot = a
        psi_dot = v * tan(delta) / Car length
    
    Updates per DT (Delta Time) for Equation1:
        x = dt ∗ x_dot + x
        y = dt ∗ y_dot + y
        v = dt * v_dot + v
        psi = dt ∗ psi_dot + psi
    
    Equation2:
        slip angle = arctan(tan(δ) * rearDistance / (frontDistance * rearDistance))
        x_dot = v * cos(psi + slip angle)
        y_dot = v * sin(psi + slip angle)
        v_dot = a
        psi_dot = v / rearDistance * sin(slip angle)
    
    Update per DT for Equation2:
        x = dt * x_dot + x
        y = dt * y_dot + y
        v = dt * v_dot + v
        psi = dt * psi_dot + psi


    */
    int numSamples = t / dT;
    for (int i=0; i<numSamples; i++) {
        float timeUpdated = time[i] + dT;
        time.push_back(timeUpdated);
        float velocityUpdated = 10;  // = velocity[i] + dT * acceleration_Input;
        velocity.push_back(velocityUpdated);

        float slipAngle = atan(tan(steerAngle_Input) * rearDistance / (frontDistance + rearDistance));
        float headingUpdated = heading[i] + dT * (velocity[i] / rearDistance * sin(slipAngle));
        heading.push_back(headingUpdated);

        float posXUpdated = positionX[i] + dT * (velocity[i] * cos(heading[i] + slipAngle));
        positionX.push_back(posXUpdated);
        float posYUpdated = positionY[i] + dT * (velocity[i] * sin(heading[i] + slipAngle));
        positionY.push_back(posYUpdated);

        cout << time[i] << "\t" << positionX[i] << "\t" << positionY[i] << "\t" << velocity[i] << "\t" << heading[i] << endl;
    }
}


void simulateDynamics() {
    /*
    Dynamics bicycle model
    */
}

void Car::writeToFile(string fileName) {
    ofstream oFile;
    oFile.open(fileName);
    oFile << "t(s)" << "\t" << "X(m)" << "\t" << "Y(m)" << "\t" << "V(m/s)" << "\t" << "Psi(rad)" << endl;

    for (int i=0; i<time.size(); i++) {
        oFile << time[i] << "\t" << positionX[i] << "\t" << positionY[i] << "\t" << velocity[i] << "\t" << heading[i] << endl;
        // oFile << setprecision(5) << time[i] << "\t" << positionX[i] << "\t" << positionY[i] << "\t" << velocity[i] << "\t" << heading[i] << endl;
    }
    oFile.close();
}
