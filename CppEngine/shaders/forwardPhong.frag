#version 450 core

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

uniform vec3 camPos;


struct PointLightToGPU {
	vec4 position;
	vec4 color;
	float luminance;
	float[3] padding;
};
layout(std430, binding = 0) buffer PointLights {
	PointLightToGPU pointLights[];
};
uniform int numLights;


in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;
in mat3 tbn;

out vec4 finalColor;

void main() {

	// calculate normal
	vec3 n;
	if (usingNormal) {
		// https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		// Normal map
		vec3 normal = texture(normalTex, fragUV).rgb * 2.0 - 1.0;
		n = normalize(tbn * normal);
	} else {
		n = normalize(fragNorm);
	}

	// diffuse and opacity
	vec4 a = texture(ambientTex, fragUV) * 0.001 * vec4(ambient, 1);
	vec4 d = texture(diffuseTex, fragUV) * vec4(diffuse, 1);

	// Quick exit if we have a fully transparent fragment
	float o = texture(alphaTex, fragUV).r;
	if (o < 0.0001) { discard; }

	vec3 albedo = vec3(a.xyz + d.xyz);

	// specular
	vec3 spec = texture(specularTex, fragUV).rgb * specular;
	float specExp = texture(specularHighLightTex, fragUV).r * specularExp;

	// eye
    vec3 eye = normalize(camPos-fragPos);


	vec3 outColor = vec3(0, 0, 0);
	// For each light, run through and add up calculations
	for (int i = 0; i < numLights; i++) {
		vec3 lightDir = normalize(pointLights[i].position.xyz - fragPos);

		float ndotL = max(dot(n, lightDir), 0.0);
		if (ndotL > 0.0001) {
			// diffuse
			vec3 diffuseColor = albedo * pointLights[i].color.rgb * ndotL;

			// specular
			vec3 h = normalize(lightDir + eye);
			float exponent = pow(max(dot(h, n), 0.0), specExp);
			vec3 specularColor = spec * exponent;

			// attenuation
			float dist = length(pointLights[i].position.xyz - fragPos);
			float attenuation = pointLights[i].luminance / (1 + 1 * dist + 2 * dist * dist);

			if (attenuation < 0.01) { attenuation = 0; }

			outColor += diffuseColor * attenuation; // diffuse
			outColor += specularColor * attenuation; // specular
		}
	}
    
    //finalColor = vec4(outColor, o);
    finalColor = vec4(outColor, o);
}