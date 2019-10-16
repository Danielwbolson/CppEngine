
#include "MeshRenderer.h"

MeshRenderer::MeshRenderer(Mesh* m, Material* mat) {
    componentType = "meshRenderer";

    mesh = m;
    material = mat;
}

MeshRenderer::~MeshRenderer() {
	for (int i = 0; i < 4; i++) {
		glDeleteBuffers(1, &vbo[i]);
	}
	glDeleteVertexArrays(1, &vao);

	delete material;
	delete mesh;
}

MeshRenderer* MeshRenderer::clone() const {
    return new MeshRenderer(*this);
}