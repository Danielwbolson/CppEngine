
#include "Material.h"
#include "Utility.h"
#include "Texture.h"

Material::Material() {}

Material::~Material() {
	ambientTexture = nullptr;
	diffuseTexture = nullptr;
	specularTexture = nullptr;
	specularHighLightTexture = nullptr;
	bumpTexture = nullptr;
	displacementTexture = nullptr;
	alphaTexture = nullptr;
}

Material::Material(const std::string& filename) {
	this->filename = filename;
}
