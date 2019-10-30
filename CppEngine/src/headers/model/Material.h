
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

	glm::vec3 ambient = glm::vec3(0.1, 0.1, 0.1);
	glm::vec3 diffuse = glm::vec3(1, 1, 1);
	glm::vec3 specular = glm::vec3(.1, .1, .1);

	float specularExponent = 10;
	float opacity = 1;
	int illum = 2;

	Texture* ambientTexture				= nullptr; // map_ka
	Texture* diffuseTexture				= nullptr; // map_kd
	Texture* specularTexture			= nullptr; // map_ks
	Texture* specularHighLightTexture	= nullptr; // map_ns
	Texture* bumpTexture				= nullptr; // map_bump
	Texture* displacementTexture		= nullptr; // disp
	Texture* alphaTexture				= nullptr; // map_d

	int ambientIndex			= -1;
	int diffuseIndex			= -1;
	int specularIndex			= -1;
	int specularHighLightIndex	= -1;
	int bumpIndex				= -1;
	int displacementIndex		= -1;
	int alphaIndex				= -1;

	Shader* shader = nullptr;


    Material();
	~Material();

    Material(const std::string& filename);

    Material operator=(const Material&);
	bool operator==(const Material&) const;
};

#endif