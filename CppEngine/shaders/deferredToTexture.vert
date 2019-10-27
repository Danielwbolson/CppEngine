#version 150 core

in vec3 inPos;
in vec3 inNorm;
in vec2 inUV;

out vec3 fragNorm;
out vec3 fragPos;
out vec2 fragUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
   gl_Position = proj * view * model * vec4(inPos,1.0);
   fragPos = (view * model * vec4(inPos,1.0)).xyz;
   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNorm,0.0);
   fragNorm = normalize(norm4.xyz);
   fragUV = inUV;
}