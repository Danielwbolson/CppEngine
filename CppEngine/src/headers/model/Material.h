
#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "glad/glad.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include <string>

class Material {
public:

	glm::vec3 color;
	glm::vec3 c_ambient, c_diffuse, c_specular;

    Material();
	~Material();

    Material(const glm::vec3&, const glm::vec3&, const glm::vec3&, const glm::vec3&, 
        const std::string&, const std::string&);

    Material operator=(const Material&);
	bool operator==(const Material&) const;

    GLuint shaderProgram;
};

#endif