#version 450 core

layout (location = 0) out vec4 finalColor;

uniform sampler2D ambientTex;
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D specularHighLightTex;
uniform sampler2D bumpTex;
uniform sampler2D normalTex;
uniform sampler2D dispTex;
uniform sampler2D alphaTex;

uniform sampler2D depthMap;
const float span = 1.0 / 4096.0;

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

uniform vec3 lightDir;
uniform vec3 lightCol;
uniform mat4 lightProjView;

in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;
in mat3 tbn;

vec3 calculatePointLights(vec3, vec3, vec4, vec3, float);
vec3 calculateDirectionalLight(vec3, vec3, vec4, vec3, float);
float calculateShadow(vec4);

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

	vec4 fragPosLightSpace = (lightProjView * vec4(fragPos, 1.0));
	float shadow = calculateShadow(fragPosLightSpace);
	vec3 directionalLightCol = (1.0 - shadow) * calculateDirectionalLight(eye, n, d, spec, specExp);

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

	float ndotL = max(dot(n, -lightDir), 0.0);
	if (ndotL > 0.0) {
		// diffuse
		vec3 diffuseColor = lightCol * d.xyz * ndotL;

		// specular
		vec3 h = normalize(-lightDir + eye);
		float exponent = pow(max(dot(h, n), 0.0), specExp);
		vec3 specularColor = spec * exponent;

        outColor = diffuseColor + specularColor; // diffuse
    }
    
    return outColor;
}

float calculateShadow(vec4 fragPosLightSpace) {
	// Calculate where our fragment is located on the screen
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	
	float shadow = 0;
	for (int i = 0; i < 5; i++) {
		float x = projCoords.x + span * (i - 2);
		for (int j = 0; j < 5; j++) {
			float y = projCoords.y + span * (j - 2);
			float textureDepth = texture(depthMap, vec2(x, y)).r;

			shadow += textureDepth < projCoords.z - 0.001 ? 1.0 : 0.0;
		}
	}

	return shadow / 25.0f;
}