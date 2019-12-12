#version 450 core

layout (location = 0) out vec4 finalColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecularExp;

uniform sampler2D depthMap;
const float span = 1.0 / 4096.0;

uniform vec3 lightDir;
uniform vec3 lightCol;
uniform mat4 lightProjView;

uniform vec3 camPos;

in vec2 UV;


float calculateShadow(vec4);

void main() {

    vec3 fragPos      = texture(gPosition, UV).rgb;
    vec3 normal       = normalize(texture(gNormal, UV).rgb);
    vec3 diffuseColor = texture(gDiffuse, UV).rgb;
    vec4 specExp      = texture(gSpecularExp, UV);
    
    // eye
    vec3 eye = normalize(camPos-fragPos);
    vec3 outColor = vec3(0, 0, 0);

	vec4 fragPosLightSpace = (lightProjView * vec4(fragPos, 1.0));

    float ndotL = max(dot(normal, -lightDir), 0.0);
    if (ndotL > 0.0) {
        // diffuse
        vec3 diffuse = lightCol * diffuseColor * ndotL;

        // specular
        vec3 h = normalize(-lightDir + eye);
        float spec = pow(max(dot(h, normal), 0.0), specExp.a);
        vec3 specular = specExp.rgb * spec;

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