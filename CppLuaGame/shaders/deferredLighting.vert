#version 420 core

in vec3 inPos;

out vec2 UV;
out vec4 outPos;

uniform mat4 pvm;

void main() {
    gl_Position = pvm * vec4(inPos, 1.0);
    outPos = gl_Position;
}