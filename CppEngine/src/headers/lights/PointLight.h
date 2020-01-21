
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
		// 0.01 = lum / (1 + 2 * radius + 2 * radius * radius);
		float a = 2;
		float b = 1;
		float c = 0.01f / lum;
		c = 1.0f / c;
		c = 1 - c; // Now on right side of equation
		radius = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
	}

    PointLight operator=(const PointLight& p) {
        if (this == &p) return *this;
        this->color = p.color;
        this->position = p.position;
        return *this;
    }
};

#endif
