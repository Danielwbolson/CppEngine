
#ifndef MATERIAL_H_
#define MATERIAL_H_

#define GLM_FORCE_RADIANS //ensure we are using radians
#include "../glad/glad.h"  //Include order can matter here
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include "Utility.h"

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