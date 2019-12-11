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
	vec3 color;
	float luminance;
};
layout(std430, binding = 0) buffer PointLights {
	PointLightToGPU pointLights[];
};
uniform int numLights;

uniform vec3 dirLightDir;
uniform vec3 dirLightCol;


in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;
in mat3 tbn;

out vec4 finalColor;

vec3 calculatePointLights(vec3, vec3, vec4, vec3, float);
vec3 calculateDirectionalLight(vec3, vec3, vec4, vec3, float);

void main() {

	// Quick exit if we have a fully transparent fragment
	float o = texture(alphaTex, fragUV).r;
	if (o < 0.0001) { discard; }


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
	vec4 a = texture(ambientTex, fragUV) * 0.1 * vec4(ambient, 1);
	vec4 d = texture(diffuseTex, fragUV) * vec4(diffuse, 1);

	// specular
	vec3 spec = texture(specularTex, fragUV).rgb * specular;
	float specExp = texture(specularHighLightTex, fragUV).r * specularExp;

	// get vector to camera
    vec3 eye = normalize(camPos-fragPos);

	// Calculate total color from lights
	vec3 pointLightCol = calculatePointLights(eye, n, d, spec, specExp);
	vec3 directionalLightCol = calculateDirectionalLight(eye, n, d, spec, specExp);

    finalColor = vec4(pointLightCol + directionalLightCol + a.xyz, o);
}

vec3 calculatePointLights(vec3 eye, vec3 n, vec4 d, vec3 spec, float specExp) {

	vec3 outColor = vec3(0, 0, 0);

	// For each pointLight, run through and add up calculations
	for (int i = 0; i < numLights; i++) {
		vec3 lightDir = normalize(pointLights[i].position.xyz - fragPos);

		float ndotL = max(dot(n, lightDir), 0.0);
		if (ndotL > 0.0) {
			// diffuse
			vec3 diffuseColor = d.xyz * pointLights[i].color * ndotL;

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

	return outColor;
}

vec3 calculateDirectionalLight(vec3 eye, vec3 n, vec4 d, vec3 spec, float specExp) {

	vec3 outColor = vec3(0, 0, 0);

	float ndotL = max(dot(n, -dirLightDir), 0.0);
	if (ndotL > 0.0) {
		// diffuse
		vec3 diffuseColor = dirLightCol * d.xyz * ndotL;

		// specular
		vec3 h = normalize(-dirLightDir + eye);
		float exponent = pow(max(dot(h, n), 0.0), specExp);
		vec3 specularColor = spec * exponent;

		outColor += diffuseColor; // diffuse
		outColor += specularColor; // specular
	}

	return outColor;
}