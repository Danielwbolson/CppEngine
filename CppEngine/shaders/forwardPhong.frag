#version 450 core

#define MAX_LIGHTS_PER_TILE 1023
#define WORK_GROUP_SIZE 16

layout (location = 0) out vec4 finalColor;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D specularHighLightTex;
uniform sampler2D bumpTex;
uniform sampler2D normalTex;
uniform sampler2D dispTex;
uniform sampler2D alphaTex;

uniform sampler2D depthMap;
const float span = 1.0 / 16384.0;

uniform bool usingBump;
uniform bool usingNormal;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float specularExp;
uniform float opacity;


struct PointLightToGPU {
	vec4 position_and_radius;
	vec4 color_and_luminance;
};
layout(std430, binding = 0) buffer PointLights {
	PointLightToGPU pointLights[];
};

// each tile supports 1023 unique point lights
struct Tile {
	uint pointLightIndices[MAX_LIGHTS_PER_TILE]; // 1023 point lights supported
	uint numLights;
};
layout(std430, binding = 1) readonly buffer LightTiles {
	Tile lightTiles[];
};

uniform vec3 camPos;
uniform ivec2 numTiles;

uniform vec3 directionalLightDir;
uniform vec3 directionalLightCol;
uniform mat4 directionalLightProjView;

in vec4 outPos;

in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;
in mat3 tbn;

vec3 calculatePointLights(vec3, vec3, vec4, vec3, float, vec2);
vec3 calculateDirectionalLight(vec3, vec3, vec4, vec3, float);
float calculateShadow(vec4);

void main() {

    vec3 normalizedFragPos = outPos.xyz / outPos.w;
    vec2 screenCoords = 0.5 * (normalizedFragPos.xy + vec2(1.0));

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

	// diffuse 
	vec4 d = texture(diffuseTex, fragUV) * vec4(diffuse, 1);

	// specular
	vec3 spec = texture(specularTex, fragUV).rgb * specular;
	float specExp = texture(specularHighLightTex, fragUV).r * specularExp;

	// get vector to camera
    vec3 eye = normalize(camPos-fragPos);

	// Calculate total color from lights
	vec3 pointLightCol = calculatePointLights(eye, n, d, spec, specExp, screenCoords);

	vec4 fragPosLightSpace = (directionalLightProjView * vec4(fragPos, 1.0));
	float shadow = calculateShadow(fragPosLightSpace);
	vec3 directionalLightCol = /*vec3(0, 0, 0);*/ (1.0 - shadow) * calculateDirectionalLight(eye, n, d, spec, specExp);

    finalColor = vec4(pointLightCol + directionalLightCol, o);
}

vec3 calculatePointLights(vec3 eye, vec3 n, vec4 d, vec3 spec, float specExp, vec2 screenCoords) {

	// First, calculate which tile this fragment is in
	ivec2 tileCoord = ivec2(screenCoords.x * numTiles.x, screenCoords.y * numTiles.y);
	int oneDimTileCoord = tileCoord.y * numTiles.x + tileCoord.x;
	
	vec3 outColor = vec3(0, 0, 0);

	// For each pointLight, run through and add up calculations
	for (int i = 0; i < lightTiles[oneDimTileCoord].numLights; i++) {
		uint index = lightTiles[oneDimTileCoord].pointLightIndices[i];

		vec3 lightDir = normalize(pointLights[index].position_and_radius.xyz - fragPos);

		float ndotL = max(dot(n, lightDir), 0.0);
		if (ndotL > 0.0) {
			// diffuse
			vec3 diffuseColor = d.rbg * pointLights[index].color_and_luminance.rgb * ndotL;

			// specular
			vec3 h = normalize(lightDir + eye);
			float exponent = pow(max(dot(h, n), 0.0), specExp);
			vec3 specularColor = spec * exponent;

			// attenuation
			float dist = length(pointLights[index].position_and_radius.xyz - fragPos);
			float attenuation = pointLights[index].color_and_luminance.a / (1 + 1 * dist + 2 * dist * dist);

			if (attenuation < 0.01) { attenuation = 0; }

			outColor += diffuseColor * attenuation; // diffuse
			outColor += specularColor * attenuation; // specular
		}
	}

	return outColor;
}

vec3 calculateDirectionalLight(vec3 eye, vec3 n, vec4 d, vec3 spec, float specExp) {

	vec3 outColor = vec3(0, 0, 0);

	float ndotL = max(dot(n, -directionalLightDir), 0.0);
	if (ndotL > 0.0) {
		// diffuse
		vec3 diffuseColor = directionalLightCol * d.xyz * ndotL;

		// specular
		vec3 h = normalize(-directionalLightDir + eye);
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

			shadow += textureDepth < projCoords.z - 0.00001 ? 1.0 : 0.0;
		}
	}

	return shadow / 25.0f;
}