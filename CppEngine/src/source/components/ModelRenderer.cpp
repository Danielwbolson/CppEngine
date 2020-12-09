
#include "ModelRenderer.h"

#include "Model.h"

ModelRenderer::ModelRenderer(Model* m) {
    componentType = "modelRenderer";

    model = m;
	m->modelRenderer = this;
	vaos.resize(model->meshes.size());
	vbos.resize(model->meshes.size());

	for (int i = 0; i < vaos.size(); i++) {
		std::array<GLuint, 6> arr = { 0, 0, 0, 0, 0, 0 };
		vbos[0] = arr;
	}

	numMeshes = static_cast<int>(vaos.size());
}

ModelRenderer::~ModelRenderer() {
	for (int i = 0; i < vaos.size(); i++) {
		for (int j = 0; j < vbos[i].size(); j++) {
			glDeleteBuffers(1, &vbos[i][j]);
		}
		glDeleteVertexArrays(1, &vaos[i]);
	}

	vaos.clear();
	vbos.clear();

	MemoryManager::Free(model);
}

ModelRenderer* ModelRenderer::clone() const {
	return MemoryManager::Allocate<ModelRenderer>(*this);
}