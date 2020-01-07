
#ifndef SHADER_H_
#define SHADER_H_

#include <string>

#include "glad/glad.h"
#include "Utility.h"

class Shader {
public:
	GLuint shaderProgram;

	Shader() {}
	Shader(const std::string& vert, const std::string& frag);

};

#endif // SHADER_H_