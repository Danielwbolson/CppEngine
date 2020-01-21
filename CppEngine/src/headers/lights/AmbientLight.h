
#ifndef AMBIENT_LIGHT_H_
#define AMBIENT_LIGHT_H_

#include "Light.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

struct AmbientLight {
	std::string type;
	float lum;

    glm::vec4 color;

    AmbientLight(const glm::vec4& c) : color(c) {
		type = "ambient";

		lum = .6f * color.g + .3f * color.r + .1f * color.b;
	}
};
#endif
