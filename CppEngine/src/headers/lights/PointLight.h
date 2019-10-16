
#ifndef POINT_LIGHT_H_
#define POINT_LIGHT_H_

#include "Light.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

struct PointLight : public Light {
    glm::vec4 color;
    glm::vec4 position;

    PointLight(const glm::vec4& c, const glm::vec4& p) : color(c), position(p) {}

    PointLight operator=(const PointLight& p) {
        if (this == &p) return *this;
        this->color = p.color;
        this->position = p.position;
        return *this;
    }
};

#endif
