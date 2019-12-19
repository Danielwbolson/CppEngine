#version 450 core

layout (location = 0) out vec4 finalColor;

uniform sampler2D gNormal;
uniform usampler2D gDiffuseSpec;
uniform sampler2D gDepth;

uniform sampler2D depthMap;
const float span = 1.0 / 4096.0;

uniform vec3 lightDir;
uniform vec3 lightCol;
uniform mat4 lightProjView;

uniform vec3 camPos;

uniform mat4 invProj;
uniform mat4 invView;

in vec2 UV;


float calculateShadow(vec4);

void main() {

    vec3 normal       = normalize(texture(gNormal, UV).rgb);
    uvec4 diffuseSpec = texture(gDiffuseSpec, UV);
    float depth       = texture(gDepth, UV).r;

	// Calculate fragment position from depth
	float z = 2.0 * depth - 1;
	vec4 clipSpacePos = vec4(UV * 2.0 - 1.0, z, 1.0);
	vec4 viewSpacePos = invProj * clipSpacePos;
	viewSpacePos /= viewSpacePos.w;
	vec3 fragPos = (invView * viewSpacePos).xyz;
    
    // eye
    vec3 eye = normalize(camPos-fragPos);
    vec3 outColor = vec3(0, 0, 0);

	vec4 fragPosLightSpace = (lightProjView * vec4(fragPos, 1.0));

	vec3 diffuseColor; vec3 specularColor;

    float ndotL = max(dot(normal, -lightDir), 0.0);
    if (ndotL > 0.0) {
        // diffuse
		diffuseColor.x = float(uint(diffuseSpec.x) & 0xFF) / 255.0;
		diffuseColor.y = float(uint(diffuseSpec.y) & 0xFF) / 255.0;
		diffuseColor.z = float(uint(diffuseSpec.z) & 0xFF) / 255.0;
        vec3 diffuse = lightCol * diffuseColor * ndotL;

        // specular
        vec3 h = normalize(lightDir + eye);
        float spec = pow(max(dot(h, normal), 0.0), diffuseSpec.a);
		
		specularColor.x = float(uint(diffuseSpec.x) >> 8) / 255.0;
		specularColor.y = float(uint(diffuseSpec.y) >> 8) / 255.0;
		specularColor.z = float(uint(diffuseSpec.z) >> 8) / 255.0;
        vec3 specular = specularColor * spec;

		float shadow = calculateShadow(fragPosLightSpace);
        outColor = (1.0 - shadow) * (diffuse + specular); // diffuse
    }
    
    finalColor = vec4(outColor, 1.0);
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