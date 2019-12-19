#version 450 core

layout (location = 0) out vec3 gNormal;
layout (location = 1) out uvec4 gDiffuseSpec;

uniform sampler2D ambientTex;
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D specularHighLightTex;
uniform sampler2D bumpTex;
uniform sampler2D normalTex;
uniform sampler2D dispTex;

uniform bool usingNormal;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float specularExp;

in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;
in mat3 tbn;

void main() {

	if (usingNormal) {
		// https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		// Normal map
		vec3 normal = texture(normalTex, fragUV).rgb * 2.0 - 1.0;
		gNormal = normalize(tbn * normal);
	} else {
		gNormal = normalize(fragNorm);
	}

	// Pack our two 8bit diffuse/ambient and specular into 1 16bit value
	vec4 a = texture(ambientTex, fragUV) * 0.3 * vec4(ambient, 1);
	vec4 d = texture(diffuseTex, fragUV) * vec4(diffuse, 1);
	uint dx = uint(floor(255.0 * d.x));
	uint dy = uint(floor(255.0 * d.y));
	uint dz = uint(floor(255.0 * d.z));

	vec3 s = texture(specularTex, fragUV).rgb * specular;
	uint sx = uint(floor(255.0 * s.x));
	uint sy = uint(floor(255.0 * s.y));
	uint sz = uint(floor(255.0 * s.z));

	uint sE = uint(specularExp * texture(specularHighLightTex, fragUV).r);

	gDiffuseSpec = uvec4(dx | (sx << 8), dy | (sy << 8), dz | (sz << 8), sE);

}