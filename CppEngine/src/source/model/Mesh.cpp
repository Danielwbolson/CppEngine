
#include "Mesh.h"
#include "Globals.h"

Mesh::Mesh() {
	bounds = nullptr;
}

Mesh::~Mesh() {
	memoryManager->Free(bounds);
}

Mesh::Mesh(std::vector<glm::vec3> p, std::vector<glm::vec3> n, std::vector<glm::vec2> uv, std::vector<unsigned int> ind) {
    positions = p;
    normals = n;
    uvs = uv;
    indices = ind;

	bounds = nullptr;
}