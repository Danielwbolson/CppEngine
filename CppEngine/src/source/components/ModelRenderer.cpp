
#include "ModelRenderer.h"

#include "Model.h"

ModelRenderer::ModelRenderer(Model* m) {
    componentType = "modelRenderer";

    model = m;
	vaos.resize(model->meshes.size());
	vbos.resize(model->meshes.size());

	ambientTextures.resize(model->meshes.size());
	diffuseTextures.resize(model->meshes.size());
	specularTextures.resize(model->meshes.size());
	specularHighLightTextures.resize(model->meshes.size());
	bumpTextures.resize(model->meshes.size());
	displacementTextures.resize(model->meshes.size());
	alphaTextures.resize(model->meshes.size());

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

		glDeleteTextures(1, &ambientTextures[i]);
		glDeleteTextures(1, &diffuseTextures[i]);
		glDeleteTextures(1, &specularTextures[i]);
		glDeleteTextures(1, &specularHighLightTextures[i]);
		glDeleteTextures(1, &bumpTextures[i]);
		glDeleteTextures(1, &displacementTextures[i]);
		glDeleteTextures(1, &alphaTextures[i]);
	}

	vaos.clear();
	vbos.clear();

	ambientTextures.clear();
	diffuseTextures.clear();
	specularTextures.clear();
	specularHighLightTextures.clear();
	bumpTextures.clear();
	displacementTextures.clear();
	alphaTextures.clear();

	delete model;
}

ModelRenderer* ModelRenderer::clone() const {
    return new ModelRenderer(*this);
}