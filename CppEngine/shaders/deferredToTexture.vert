#version 150 core

in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexcoord;

out vec3 Color;
out vec3 vertNormal;
out vec3 pos;
out vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 inColor;

void main() {
   Color = inColor;
   gl_Position = proj * view * model * vec4(inPosition,1.0);
   pos = (view * model * vec4(inPosition,1.0)).xyz;
   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);
   vertNormal = normalize(norm4.xyz);
   texcoord = inTexcoord;
}