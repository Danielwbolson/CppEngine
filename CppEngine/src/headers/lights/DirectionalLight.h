
#ifndef DIRECTIONAL_LIGHT_H_
#define DIRECTIONAL_LIGHT_H_

#include "Light.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

struct DirectionalLight : public Light {
    glm::vec4 color;
    glm::vec4 direction;

    DirectionalLight(const glm::vec4& c, const glm::vec4& d) : color(c), direction(d) {
		type = "directionalLight";

		lum = .6f * color.g + .3f * color.r + .1f * color.b;
	}
};

#endif
