
#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "glad/glad.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include <string>

class Texture;

class Material {
public:

	std::string filename;
	std::string vertFile;
	std::string fragFile;

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

	GLuint shaderProgram;


    Material();
	~Material();

    Material(const std::string& filename, const std::string&, const std::string&);

    Material operator=(const Material&);
	bool operator==(const Material&) const;
};

#endif