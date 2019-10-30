
#include "Bounds.h"

Bounds::Bounds() : 
	minX(0), minY(0), minZ(0), 
	maxX(0), maxY(0), maxZ(0), 
	points(std::vector<glm::vec4>(8)) {

}

Bounds::Bounds(const float minVx, const float minVy, const float minVz,
	const float maxVx, const float maxVy, const float maxVz) :
	minX(minVx), minY(minVy), minZ(minVz),
	maxX(maxVx), maxY(maxVy), maxZ(maxVz),
	points(std::vector<glm::vec4>(8)) {

	points[0] = glm::vec4(maxX, maxY, maxZ, 1);
	points[1] = glm::vec4(maxX, minY, maxZ, 1);
	points[2] = glm::vec4(maxX, maxY, minZ, 1);
	points[3] = glm::vec4(maxX, minY, minZ, 1);
	points[4] = glm::vec4(minX, maxY, maxZ, 1);
	points[5] = glm::vec4(minX, minY, maxZ, 1);
	points[6] = glm::vec4(minX, maxY, minZ, 1);
	points[7] = glm::vec4(minX, minY, minZ, 1);

}

Bounds::Bounds(const Bounds& b) {
	this->minX = b.minX;
	this->minY = b.minY;
	this->minZ = b.minZ;

	this->maxX = b.maxX;
	this->maxY = b.maxY;
	this->maxZ = b.maxZ;

	this->points = b.points;
}

Bounds* Bounds::clone() const {
	return new Bounds(*this);
}

Bounds& Bounds::operator=(const Bounds& b) {
	minX = b.minX;
	minY = b.minY;
	minZ = b.minZ;
	maxX = b.maxX;
	maxY = b.maxY;
	maxZ = b.maxZ;

	points = b.points;

	return *this;
}

void Bounds::Init() {
	points[0] = glm::vec4(maxX, maxY, maxZ, 1);
	points[1] = glm::vec4(maxX, minY, maxZ, 1);
	points[2] = glm::vec4(maxX, maxY, minZ, 1);
	points[3] = glm::vec4(maxX, minY, minZ, 1);
	points[4] = glm::vec4(minX, maxY, maxZ, 1);
	points[5] = glm::vec4(minX, minY, maxZ, 1);
	points[6] = glm::vec4(minX, maxY, minZ, 1);
	points[7] = glm::vec4(minX, minY, minZ, 1);
}

glm::vec3 Bounds::Max(const glm::mat4 t) {

	float newMaxX = -INFINITY;
	float newMaxY = -INFINITY;
	float newMaxZ = -INFINITY;

	for (int i = 0; i < points.size(); i++) {
		glm::vec4 p = t * points[i];

		if (p.x > newMaxX) newMaxX = p.x;
		if (p.y > newMaxY) newMaxY = p.y;
		if (p.z > newMaxZ) newMaxZ = p.z;
	}

	return glm::vec3(newMaxX, newMaxY, newMaxZ);

}

glm::vec3 Bounds::Min(const glm::mat4 t) {

	float newMinX = INFINITY;
	float newMinY = INFINITY;
	float newMinZ = INFINITY;

	for (int i = 0; i < points.size(); i++) {
		glm::vec4 p = t * points[i];

		if (p.x < newMinX) newMinX = p.x;
		if (p.y < newMinY) newMinY = p.y;
		if (p.z < newMinZ) newMinZ = p.z;
	}

	return glm::vec3(newMinX, newMinY, newMinZ);

}