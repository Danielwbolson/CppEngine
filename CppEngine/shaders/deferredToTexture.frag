#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gDiffuse;
layout (location = 3) out vec4 gSpecularExp;

uniform sampler2D ambientTex;
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D specularHighLightTex;
uniform sampler2D bumpTex;
uniform sampler2D dispTex;
uniform sampler2D alphaTex;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float specularExp;
uniform float opacity;

in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;

void main() {

    gPosition = fragPos;
    gNormal = normalize(fragNorm);

	vec4 a = vec4(ambient, 1) * texture(ambientTex, fragUV);
	vec4 d = vec4(diffuse, 1) * texture(diffuseTex, fragUV);
	float o = opacity * texture(alphaTex, fragUV).r;

	gDiffuse = vec3(a.xyz + d.xyz);

	vec4 s = vec4(specular, 1) * texture(specularTex, fragUV);
	float sE = specularExp * texture(specularHighLightTex, fragUV).r;

	gSpecularExp = vec4(s.xyz, sE);
}