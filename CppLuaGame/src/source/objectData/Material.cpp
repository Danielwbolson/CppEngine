
#include "Material.h"
#include "Utility.h"

Material::Material() {}

    Material::Material(const glm::vec3& c, const glm::vec3& a, const glm::vec3& d, const glm::vec3& s, 
        const std::string& vert, const std::string& frag) {
    this->color = c;
    this->c_ambient = a;
    this->c_diffuse = d;
    this->c_specular = s;

    this->shaderProgram = util::initShaderFromFiles(vert, frag);
}

Material Material::operator=(const Material& m) {
    if (this == &m) return *this;

    this->shaderProgram = m.shaderProgram;

    this->color = m.color;
    this->c_ambient = m.Ambient();
    this->c_diffuse = m.Diffuse();
    this->c_specular = m.Specular();

    return *this;
}