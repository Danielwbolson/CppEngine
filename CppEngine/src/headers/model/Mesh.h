
#ifndef MESH_H_
#define MESH_H_

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include "Bounds.h"

#include <vector>

class Mesh {
public:
    std::string name;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;

	Bounds* bounds;

    Mesh();
	~Mesh();
    Mesh(std::vector<glm::vec3>, std::vector<glm::vec3>, std::vector<glm::vec2>, std::vector<unsigned int>);

    std::string ComponentType() const { return "mesh"; }

};
#endif
