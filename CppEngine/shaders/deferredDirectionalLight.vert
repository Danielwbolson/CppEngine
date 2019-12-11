#version 450 core

in vec2 inPos;

out vec2 UV;

void main() {
	UV = inPos.xy * 0.5 + 0.5;
	gl_Position = vec4(inPos.xy, 0, 1.0);
}