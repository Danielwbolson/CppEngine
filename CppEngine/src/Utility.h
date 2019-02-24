
#ifndef UTILITY_H_
#define UTILITY_H_

#include "../glad/glad.h"  //Include order can matter here

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace util {

    static void loadShader(GLuint shaderID, const GLchar* shaderSource) {
        glShaderSource(shaderID, 1, &shaderSource, NULL);
        glCompileShader(shaderID);

        //Let's double check the shader compiled 
        GLint status;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status); //Check for errors
        if (!status) {
            char buffer[1024]; glGetShaderInfoLog(shaderID, 1024, NULL, buffer);
            printf("Shader Compile Failed. Info:\n\n%s\n", buffer);
        }
    }

    static const std::string fileToString(std::string filename) {
        std::string f = "./shaders/" + filename;
        std::ifstream file(f);
        std::stringstream buf;
        buf << file.rdbuf();

        return buf.str();
    }

    static GLuint initShaderFromFiles(const std::string& vert, const std::string& frag) {
        // Vert and frag shaders, compiled from file
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const std::string vertString = util::fileToString(vert);
        const GLchar* vertexSource = vertString.c_str();
        util::loadShader(vertexShader, vertexSource);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const std::string fragString = util::fileToString(frag);
        const GLchar* fragmentSource = fragString.c_str();
        util::loadShader(fragmentShader, fragmentSource);

        //Join the vertex and fragment shaders together into one program
        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        //glBindFragDataLocation(shaderProgram, 0, "outColor"); // set output
        glLinkProgram(shaderProgram); //run the linker

        return shaderProgram;
    }
}

#endif