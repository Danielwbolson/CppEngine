#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gDiffuse;
layout (location = 3) out vec4 gSpecularExp;

in vec3 Color;
in vec3 vertNormal;
in vec3 pos;
in vec2 texcoord;

void main() {
    vec3 color = Color;
    
    gPosition = pos;
    gNormal = normalize(vertNormal);
    gDiffuse = color;
    gSpecularExp = vec4(.1, .1, .1, 1000);

}