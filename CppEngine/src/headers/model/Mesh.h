
#ifndef MESH_H_
#define MESH_H_

// Mesh pieces do not use our manual dynamic memory
// Mesh data is used once and then sent to the GPU, so as long as the vector
// address is in our manual memory block, we don't need the entire chunk

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include "Bounds.h"
#include "MemoryAllocator.h"

#include <vector>

class Mesh {
public:
    std::string name;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;

	Bounds* bounds;

    Mesh();
	~Mesh();
    Mesh(std::vector<glm::vec3>, 
		std::vector<glm::vec3>, 
		std::vector<glm::vec2>,
		std::vector<unsigned int> );

    std::string ComponentType() const { return "mesh"; }

};
#endif
