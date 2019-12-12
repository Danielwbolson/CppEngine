#version 450 core

uniform sampler2D finalQuadRender;

uniform float xSpan;
uniform float ySpan;

in vec2 UV;
out vec4 finalColor;


void main() {
	finalColor = texture(finalQuadRender, UV);
}