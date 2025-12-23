#include "Entity.h"
#include "../Loader.h"
#include "../shaders/RectShader.h"


// constructor
// ------------------------------------------------------------------------
Entity::Entity(Loader* loader, RectShader* rectShader) : loader(loader), rectShader(rectShader) {}

// destructor
// ------------------------------------------------------------------------
// default destructor is used. 

// getter
// ------------------------------------------------------------------------
float Entity::getPosX() const noexcept { return pos_m.x; }
float Entity::getPosY() const noexcept { return pos_m.y;}
float Entity::getYaw() const noexcept { return yaw; }
float Entity::getWidth() const noexcept { return width_m; };
float Entity::getLength() const noexcept { return length_m; };
const std::array<float, 4>& Entity::getColor() const noexcept { return color; }

// setter
// ------------------------------------------------------------------------
void Entity::setPos(const Position2D& newPos) { pos_m = newPos;}
void Entity::setYaw(const float newYaw) { yaw = newYaw;}
void Entity::setWidth(const float newWidth) { width_m = newWidth; }
void Entity::setLength(const float newLength) { length_m = newLength; }
void Entity::setColor(const std::array<float, 4>& newColor) { color = newColor;}
