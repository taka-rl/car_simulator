#ifndef ENTITY_H
#define ENTITY_H

#include <array>


struct State { float x, y; };

// forward declarations at global scope
class Loader;
class RectShader;

class Entity {

public:
    State pos_m{0.f, 0.f};
    float yaw{0.f}, width_m{0.f}, height_m{0.f};
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
    float getHeight() const noexcept;
    const std::array<float, 4>& getColor() const noexcept;
    
    // setter
    // ------------------------------------------------------------------------
    void setPos(const State& newPos);
    void setYaw(const float newYaw);
    void setWidth(const float newWidth);
    void setHeight(const float newHeight);
    void setColor(const std::array<float, 4>& newColor);
};
#endif