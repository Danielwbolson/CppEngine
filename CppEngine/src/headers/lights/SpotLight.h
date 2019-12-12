
#ifndef SPOT_LIGHT_H_
#define SPOT_LIGHT_H_

#include "Light.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

struct SpotLight : public Light {
    glm::vec4 color;
    glm::vec4 position;
    glm::vec4 direction;
    float theta;
    float phi;

    SpotLight(const glm::vec4& c, const glm::vec4& pos, const glm::vec4& d, const float& t, const float& p) : 
        color(c), position(pos), direction(d), theta(t), phi(p) {
		type = "spotLight";

		lum = .6f * color.g + .3f * color.r + .1f * color.b;
	}
};

#endif
