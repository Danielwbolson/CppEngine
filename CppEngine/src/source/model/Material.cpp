
#include "Material.h"
#include "Utility.h"
#include "Texture.h"

Material::Material() {}

Material::~Material() {
	glDeleteProgram(shaderProgram);

	ambientTexture = nullptr;
	diffuseTexture = nullptr;
	specularTexture = nullptr;
	specularHighLightTexture = nullptr;
	bumpTexture = nullptr;
	displacementTexture = nullptr;
	alphaTexture = nullptr;
}

Material::Material(const std::string& filename, const std::string& vert, const std::string& frag) {
	this->filename = filename;
	vertFile = vert;
	fragFile = frag;

    this->shaderProgram = util::initShaderFromFiles(vert, frag);
}
