
#include "Material.h"
#include "Utility.h"

Material::Material() {}

Material::~Material() {
	glDeleteProgram(shaderProgram);
}

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
    this->c_ambient = m.c_ambient;
    this->c_diffuse = m.c_diffuse;
    this->c_specular = m.c_specular;

    return *this;
}

bool Material::operator==(const Material& rhs) const {
	return (
		this->color == rhs.color &&
		this->c_ambient == rhs.c_ambient &&
		this->c_diffuse == rhs.c_diffuse &&
		this->c_specular == rhs.c_specular
	);
}