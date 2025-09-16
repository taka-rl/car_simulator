#include "car.h"
#include <iostream>

using namespace std;


int main() {
    const float PI = 3.14;
    Car car1(2.0, 2.0);
    car1.setInputs(1, PI/6);
    car1.setInitialConditions(5.0, 5.0, 10.0, 0.0);
    float t1 = 300;
    string file1 = "car1.txt";

    Car car2(2.0, 2.0);
    car2.setInputs(1, PI/4);
    car2.setInitialConditions(5.0, 5.0, 10.0, 0.0);
    float t2 = 100;
    string file2 = "car2.txt";

    thread thr1(&Car::simulateKinematics, ref(car1), ref(t1));
    thread thr2(&Car::simulateKinematics, ref(car2), ref(t2));
    thr1.join();
    thr2.join();

    thread thr3(&Car::writeToFile, ref(car1), ref(file1));
    thread thr4(&Car::writeToFile, ref(car2), ref(file2));
    thr3.join();
    thr4.join();
    /*
    car1.simulateKinematics(t1);
    car2.simulateKinematics(t2);
    car1.writeToFile(file1);
    car2.writeToFile(file2);
    */
    return 0;
}
