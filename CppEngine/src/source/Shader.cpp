
#include "Shader.h"

Shader::Shader(const std::string& vert, const std::string& frag) {
	this->shaderProgram = util::initVertFragShader(vert, frag);
}
