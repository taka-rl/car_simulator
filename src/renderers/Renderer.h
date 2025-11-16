#ifndef RENDERER_H
#define RENDERER_H

#include <algorithm>

// forward declarations at global scope
class Entity;
class Loader;
class RectShader;
struct State;


class Renderer {
public: 
    
    // constructor
    // ------------------------------------------------------------------------
    Renderer(float ppm, int fbW, int fbH);

    // destructor
    // ------------------------------------------------------------------------
    // default destructor is used. 

    /** draw
     * ------------------------------------------------------------------------
     * @param[in] x_m: x coordinate of the object center
     * @param[in] y_m: y coordinate of the object center
     * @return void
    */
    void draw(const Entity& e) const;

private:
    // converts a point (the object center in meters) into an NDC position for uOffset
    // ------------------------------------------------------------------------
    State metersToNDC(float x_m, float y_m) const;

    // convcrts a full size (meters) into an NDC full size. for uScale
    // ------------------------------------------------------------------------
    State rectSizeToNDC(float width_m, float height_m) const;

    float ppm{20.f};
    int fbW{0}, fbH{0};
};
#endif