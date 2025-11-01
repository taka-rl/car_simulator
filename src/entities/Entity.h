#ifndef ENTITY_H
#define ENTITY_H

#include <array>


struct State { float x, y; };

// forward declarations at global scope
class Loader;
class RectShader;

class Entity {

public:
    State pos{0.f, 0.f};
    float yaw{0.f};
    State scale{1.f, 1.f};
    std::array<float, 4> color = {1.f, 1.f, 1.f, 1.f};  // color[r, g, b, a]
    
    Loader* loader{nullptr};            // non-owning
    RectShader* rectShader{nullptr};    // non-owning

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Entity(Loader* loader, RectShader* rectShader);

    // destructor 
    virtual ~Entity() = default;

    // getter
    float getPosX() const noexcept; 
    float getPosY() const noexcept;
    float getYaw() const noexcept;
    float getScaleX() const noexcept;
    float getScaleY() const noexcept;
    const std::array<float, 4>& getColor() const noexcept;
    
    // setter
    void setPos(const State& newPos);
    void setYaw(const float newYaw);
    void setScale(const State& newScale);
    void setColor(const std::array<float, 4>& newColor);

};
#endif