#version 150 core

in vec3 inPos;
in vec3 inNorm;
in vec2 inUV;
in vec3 inTang;
in vec3 inBitang;

out vec3 fragNorm;
out vec3 fragPos;
out vec2 fragUV;
out mat3 tbn;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
   gl_Position = proj * view * model * vec4(inPos,1.0);
   fragPos = (model * vec4(inPos,1.0)).xyz;

   // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
   fragNorm = normalize(vec3(transpose(inverse(model)) * vec4(inNorm, 0.0)));
   vec3 normal = normalize((model * vec4(inNorm, 0.0)).xyz);
   vec3 fragBitan = normalize((model * vec4(inBitang, 0.0)).xyz);
   vec3 fragTan = normalize((model * vec4(inTang, 0.0)).xyz);
   tbn = mat3(fragTan, fragBitan, fragNorm);

   fragUV = inUV;
}