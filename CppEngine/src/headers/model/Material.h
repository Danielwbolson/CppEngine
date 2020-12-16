
#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "glad/glad.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include <string>

class Texture;
class Shader;

class Material {
public:

	std::string filename;

	// Values
	glm::vec3 ambient = glm::vec3(0.1, 0.1, 0.1);
	glm::vec3 diffuse = glm::vec3(1, 1, 1);
	glm::vec3 specular = glm::vec3(.1, .1, .1);
	float specularExponent = 10;
	float opacity = 1;
	int illum = 2;
	bool useTextures = true;

	// References to textures, may be nullptr
	Texture* ambientTexture				= nullptr; // map_ka
	Texture* diffuseTexture				= nullptr; // map_kd
	Texture* specularTexture			= nullptr; // map_ks
	Texture* specularHighLightTexture	= nullptr; // map_ns
	Texture* bumpTexture				= nullptr; // map_bump
	Texture* normalTexture				= nullptr; // 
	Texture* displacementTexture		= nullptr; // disp
	Texture* alphaTexture				= nullptr; // map_d

	bool usingBump = false;
	bool usingNormal = false;
	bool usingAlpha = false;
	bool usingSpecular = false;
	bool isTransparent = false;

	// Index of GLuint for textures in shader. Actual gluint stored in AssetManager
	int ambientIndex			= -1;
	int diffuseIndex			= -1;
	int specularIndex			= -1;
	int specularHighLightIndex	= -1;
	int bumpIndex				= -1;
	int normalIndex				= -1;
	int displacementIndex		= -1;
	int alphaIndex				= -1;

	Shader* shader = nullptr;

	// Uniform location caches
	GLint uniModel;
	GLint uniView;
	GLint uniProj;

	GLint uniAmbient;
	GLint uniDiffuse;
	GLint uniSpecular;
	GLint uniSpecularExp;
	GLint uniOpacity;

	GLint uniAmbientTex;
	GLint uniDiffuseTex;
	GLint uniSpecularTex;
	GLint uniSpecularHighLightTex;
	GLint uniBumpTex;
	GLint uniNormalTex;
	GLint uniDisplacementTex;
	GLint uniAlphaTex;

	GLint uniUsingBump;
	GLint uniUsingNormal;


	// Functions
    Material();
	~Material();

    Material(const std::string& filename);

	void InitUniforms();
};

#endif