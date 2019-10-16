
#ifndef AMBIENT_LIGHT_H_
#define AMBIENT_LIGHT_H_

#include "Light.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

struct AmbientLight : public Light {
    glm::vec4 color;

    AmbientLight(const glm::vec4& c) : color(c) {}
};
#endif
