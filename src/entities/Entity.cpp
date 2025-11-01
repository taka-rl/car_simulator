#include "Entity.h"
#include "../Loader.h"
#include "../shaders/RectShader.h"


// constructor
// ------------------------------------------------------------------------
Entity::Entity(Loader* loader, RectShader* rectShader) : loader(loader), rectShader(rectShader) {}


// getter
// ------------------------------------------------------------------------
float Entity::getPosX() const noexcept { return pos.x; }
float Entity::getPosY() const noexcept { return pos.y;}
float Entity::getYaw() const noexcept { return yaw; }
float Entity::getScaleX() const noexcept { return scale.x; }
float Entity::getScaleY() const noexcept { return scale.y; }
const std::array<float, 4>& Entity::getColor() const noexcept { return color; }

// sette
// ------------------------------------------------------------------------
void Entity::setPos(const State& newPos) { pos = newPos;}
void Entity::setYaw(const float newYaw) { yaw = newYaw;}
void Entity::setScale(const State& newScale) { scale = newScale;}
void Entity::setColor(const std::array<float, 4>& newColor) { color = newColor;}


