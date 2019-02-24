
#ifndef DIRECTIONAL_LIGHT_H_
#define DIRECTIONAL_LIGHT_H_

#include "Light.h"

struct DirectionalLight : public Light {
    glm::vec4 color;
    glm::vec4 direction;

    DirectionalLight(const glm::vec4& c, const glm::vec4& d) : color(c), direction(d) {}
};

#endif
