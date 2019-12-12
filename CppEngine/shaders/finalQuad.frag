#version 450 core

uniform sampler2D finalQuadRender;

uniform float xSpan;
uniform float ySpan;

in vec2 UV;
out vec3 finalColor;


void main() {
	// gamma correction
	finalColor = pow(texture(finalQuadRender, UV).rgb, vec3(1.0 / 2.2));
}