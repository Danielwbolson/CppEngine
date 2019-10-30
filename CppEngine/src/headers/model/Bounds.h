
#ifndef BOUNDS_H_
#define BOUNDS_H_

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include <vector>

class Bounds {

public:
	Bounds();
	Bounds(const float minVx, const float minVy, const float minVz,
		const float maxVx, const float maxVy, const float maxVz);
	Bounds* clone() const;

	Bounds(const Bounds&);
	Bounds& operator=(const Bounds& b);

	void Init();

	glm::vec3 Max(const glm::mat4 t);
	glm::vec3 Min(const glm::mat4 t);

	std::vector<glm::vec4> points;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
};

#endif // BOUNDS_H_