#ifndef ENTITY_H
#define ENTITY_H

#include <array>


#include "../vehicledynamics/VehicleTypes.h"

// forward declarations at global scope
class Loader;
class RectShader;

class Entity {

public:
    Position2D pos_m{0.f, 0.f};
    float yaw{0.f}, width_m{0.f}, length_m{0.f};
    std::array<float, 4> color = {1.f, 1.f, 1.f, 1.f};  // color[r, g, b, a]
    
    Loader* loader{nullptr};            // non-owning
    RectShader* rectShader{nullptr};    // non-owning

    // constructor
    // ------------------------------------------------------------------------
    Entity(Loader* loader, RectShader* rectShader);

    // destructor
    // ------------------------------------------------------------------------
    // default destructor is used. 

    // getter
    // ------------------------------------------------------------------------
    float getPosX() const noexcept; 
    float getPosY() const noexcept;
    float getYaw() const noexcept;
    float getWidth() const noexcept;
    float getLength() const noexcept;
    const std::array<float, 4>& getColor() const noexcept;
    
    // setter
    // ------------------------------------------------------------------------
    void setPos(const Position2D& newPos);
    void setYaw(const float newYaw);
    void setWidth(const float newWidth);
    void setLength(const float newLength);
    void setColor(const std::array<float, 4>& newColor);
};
#endif