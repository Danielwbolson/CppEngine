#version 450 core

in vec3 inPos;

uniform mat4 lightProjView;
uniform mat4 model;

void main() {
	gl_Position = lightProjView * model * vec4(inPos, 1.0);
}