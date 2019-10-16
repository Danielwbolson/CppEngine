
#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "glad/glad.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include <string>

class Material {

private:
    glm::vec3 color;
    glm::vec3 c_ambient, c_diffuse, c_specular;

public:
    Material();
    Material(const glm::vec3&, const glm::vec3&, const glm::vec3&, const glm::vec3&, 
        const std::string&, const std::string&);

    glm::vec3 Color() const { return color; }
    glm::vec3 Ambient() const { return c_ambient; }
    glm::vec3 Diffuse() const { return c_diffuse; }
    glm::vec3 Specular() const { return c_specular; }

    Material operator=(const Material&);

    GLuint shaderProgram;
};

#endif