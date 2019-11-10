
#ifndef UTILITY_H_
#define UTILITY_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define GLM_FORCE_RADIANS
#include "glad/glad.h"

#include "Configuration.h"

namespace util {

	static void checkGLError(const std::string& s) {
		GLenum err;
		if ((err = glGetError()) != 0) {
			std::cout << s + " :" << err << std::endl;
		}
	}

    static void loadShader(GLuint shaderID, const GLchar* shaderSource) {
        glShaderSource(shaderID, 1, &shaderSource, NULL);
        glCompileShader(shaderID);

        //Let's double check the shader compiled 
        GLint status;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status); //Check for errors
		checkGLError("After shaderiv");
        if (!status) {
            char buffer[1024]; glGetShaderInfoLog(shaderID, 1024, NULL, buffer);
            printf("Shader Compile Failed. Info:\n\n%s\n", buffer);
        }
    }

    static const std::string fileToString(std::string filename) {
        std::string fullFile = VK_ROOT_DIR"shaders/"+ filename;
        std::ifstream file(fullFile);
        std::stringstream buf;
        buf << file.rdbuf();

        return buf.str();
    }

    static GLuint initShaderFromFiles(const std::string& vert, const std::string& frag) {
        // Vert and frag shaders, compiled from file
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const std::string vertString = util::fileToString(vert);
		if (vertString == "") {
			fprintf(stderr, "Failed to load vertex shader: %s", vert.c_str());
			exit(1);
		}
        const GLchar* vertexSource = vertString.c_str();
        util::loadShader(vertexShader, vertexSource);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const std::string fragString = util::fileToString(frag);
		if (fragString == "") {
			fprintf(stderr, "Failed to load fragment shader: %s", frag.c_str());
			exit(1);
		}
        const GLchar* fragmentSource = fragString.c_str();
        util::loadShader(fragmentShader, fragmentSource);

        //Join the vertex and fragment shaders together into one program
        GLuint shaderProgram = glCreateProgram();
		checkGLError("After shader program");
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        //glBindFragDataLocation(shaderProgram, 0, "outColor"); // set output
        glLinkProgram(shaderProgram); //run the linker

        return shaderProgram;
    }

	static void DebugMessageCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const void* userParam) {
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
	}
}

#endif