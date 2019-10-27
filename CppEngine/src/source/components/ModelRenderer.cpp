
#include "ModelRenderer.h"

#include "Model.h"

ModelRenderer::ModelRenderer(Model* m) {
    componentType = "modelRenderer";

    model = m;
	vaos.resize(model->meshes.size());
	vbos.resize(model->meshes.size());

	for (int i = 0; i < vaos.size(); i++) {
		std::array<GLuint, 4> arr = { 0, 0, 0, 0 };
		vbos[0] = arr;
	}

	numMeshes = static_cast<int>(vaos.size());
}

ModelRenderer::~ModelRenderer() {
	for (int i = 0; i < vaos.size(); i++) {
		for (int j = 0; j < 4; j++) {
			glDeleteBuffers(1, &vbos[i][j]);
		}
		glDeleteVertexArrays(1, &vaos[i]);
	}

	vaos.clear();
	vbos.clear();

	delete model;
}

ModelRenderer* ModelRenderer::clone() const {
    return new ModelRenderer(*this);
}