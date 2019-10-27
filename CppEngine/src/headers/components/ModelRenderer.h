
#ifndef MODEL_RENDERER_H_
#define MODEL_RENDERER_H_

#include <vector>
#include <array>

#include "glad/glad.h"
#include "Component.h"

class Model;
class Material;

class ModelRenderer : public Component {

public:
    // 0 : position, 1 : normals, 2 : uvs, 3 : indices
	std::vector<std::array<GLuint, 4> > vbos;
    std::vector<GLuint> vaos;

	std::vector<GLuint> ambientTextures;
	std::vector<GLuint> diffuseTextures;
	std::vector<GLuint> specularTextures;
	std::vector<GLuint> specularHighLightTextures;
	std::vector<GLuint> bumpTextures;
	std::vector<GLuint> displacementTextures;
	std::vector<GLuint> alphaTextures;

	int numMeshes;

    Model* model;

	ModelRenderer() {}
	~ModelRenderer();

	ModelRenderer(Model*);
	ModelRenderer* clone() const;

};

#endif
