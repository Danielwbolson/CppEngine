#version 450 core

in vec4 inPos;

uniform mat4 projView;
uniform mat4 model;

void main() {
	gl_Position = projView * model * inPos;
}