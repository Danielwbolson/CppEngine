
#version 150 core

in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexcoord;

const vec3 inLightDir = vec3(0,1,0);

uniform vec3 inColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 Color;
out vec3 normal;
out vec3 pos;
out vec3 eyePos;
out vec3 lightDir;
out vec2 uvs;

void main() {
   Color = inColor;
   vec4 pos4 = view * model * vec4(inPosition,1.0);
   vec4 modelPos = model * vec4(inPosition, 1.0);
   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);
   normal = norm4.xyz;
   lightDir = (view * vec4(inLightDir, 0)).xyz;
   gl_Position = proj * pos4;
};