
#include "Mesh.h"

Mesh::Mesh() {

}

Mesh::Mesh(std::vector<glm::vec3> p, std::vector<glm::vec3> n, std::vector<Vec2> uv, std::vector<unsigned int> ind) {
    pos = p;
    normals = n;
    uvs = uv;
    indices = ind;
}