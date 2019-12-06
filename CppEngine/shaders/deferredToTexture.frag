#version 450 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gDiffuse;
layout (location = 3) out vec4 gSpecularExp;

uniform sampler2D ambientTex;
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D specularHighLightTex;
uniform sampler2D bumpTex;
uniform sampler2D normalTex;
uniform sampler2D dispTex;
uniform sampler2D alphaTex;

uniform bool usingBump;
uniform bool usingNormal;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float specularExp;
uniform float opacity;

in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;
in mat3 tbn;

void main() {

    gPosition = fragPos;

	if (usingNormal) {
		// https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		// Normal map
		vec3 normal = texture(normalTex, fragUV).rgb * 2.0 - 1.0;
		gNormal = normalize(tbn * normal);
	} else {
		gNormal = normalize(fragNorm);
	}

	vec4 a = texture(ambientTex, fragUV) * 0.001 * vec4(ambient, 1);
	vec4 d = texture(diffuseTex, fragUV) * vec4(diffuse, 1);
	float o = texture(alphaTex, fragUV).r;

	gDiffuse = vec3(a.xyz + d.xyz);

	vec3 s = texture(specularTex, fragUV).rgb * specular;
	float sE = specularExp * texture(specularHighLightTex, fragUV).r;

	gSpecularExp = vec4(s, sE);
}