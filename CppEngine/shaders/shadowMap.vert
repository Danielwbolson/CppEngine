#version 450 core

in vec3 inPos;

// Matrix of light for shadows, matrix of camera for depth pre-pass
uniform mat4 projView;
uniform mat4 model;

void main() {
	gl_Position = projView * model * vec4(inPos, 1.0);
}