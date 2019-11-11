#version 150 core

in vec3 inPos;
in vec3 inNorm;
in vec2 inUV;
in vec3 inTang;
in vec3 inBitang;

out vec3 fragNorm;
out vec3 fragTan;
out vec3 fragBitan;
out vec3 fragPos;
out vec2 fragUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
   gl_Position = proj * view * model * vec4(inPos,1.0);
   fragPos = (view * model * vec4(inPos,1.0)).xyz;

   fragNorm = (view * model * vec4(inNorm, 0.0)).xyz;
   fragTan = (view * model * vec4(inTang, 0.0)).xyz;
   fragBitan = (view * model * vec4(inBitang, 0.0)).xyz;

   fragUV = inUV;
}