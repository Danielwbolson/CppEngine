
#include "Material.h"
#include "Utility.h"
#include "Texture.h"
#include "Shader.h"

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

void Material::InitUniforms() {
	uniModel = glGetUniformLocation(shader->shaderProgram, "model");
	uniView = glGetUniformLocation(shader->shaderProgram, "view");
	uniProj = glGetUniformLocation(shader->shaderProgram, "proj");

	uniAmbient = glGetUniformLocation(shader->shaderProgram, "ambient");
	uniDiffuse = glGetUniformLocation(shader->shaderProgram, "diffuse");
	uniSpecular = glGetUniformLocation(shader->shaderProgram, "specular");
	uniSpecularExp = glGetUniformLocation(shader->shaderProgram, "specularExp");
	uniOpacity = glGetUniformLocation(shader->shaderProgram, "opacity");

	uniAmbientTex = glGetUniformLocation(shader->shaderProgram, "ambientTex");
	uniDiffuseTex = glGetUniformLocation(shader->shaderProgram, "diffuseTex");
	uniSpecularTex = glGetUniformLocation(shader->shaderProgram, "specularTex");
	uniSpecularHighLightTex = glGetUniformLocation(shader->shaderProgram, "specularHighLightTex");
	uniBumpTex = glGetUniformLocation(shader->shaderProgram, "bumpTex");
	uniDisplacementTex = glGetUniformLocation(shader->shaderProgram, "dispTex");
	uniAlphaTex = glGetUniformLocation(shader->shaderProgram, "alphaTex");
}

Material::Material(const std::string& filename) {
	this->filename = filename;
}
