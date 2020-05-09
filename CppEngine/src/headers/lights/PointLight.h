
#ifndef POINT_LIGHT_H_
#define POINT_LIGHT_H_

#include "Light.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

struct PointLight {
	std::string type;
	float lum;

	glm::vec4 position; 
	glm::vec3 color;
	float radius;

    PointLight(const glm::vec3& col, const glm::vec4& pos) : color(col), position(pos) {
		type = "pointLight";

		lum = .6f * color.g + .3f * color.r + .1f * color.b;

		// Calculate radius of volume using luminence and wanted cutoff value of 0.004f
		// 0.001 = lum / (1 + 1 * radius + 2 * radius * radius);
		float a = 1;
		float b = 2;
		float lightLum = 0.005f;
		radius = sqrt(lum / (b * lightLum));
	}

    PointLight operator=(const PointLight& p) {
        if (this == &p) return *this;
        this->color = p.color;
        this->position = p.position;
        return *this;
    }
};

#endif
