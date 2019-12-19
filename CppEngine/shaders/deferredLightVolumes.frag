#version 450 core

layout (location = 0) out vec4 finalColor;

uniform sampler2D gNormal;
uniform usampler2D gDiffuseSpec;
uniform sampler2D gDepth;

uniform vec3 lightPos;
uniform vec3 lightCol;
uniform float lightLum;

uniform mat4 invProj;
uniform mat4 invView;

uniform vec3 camPos;

in vec4 outPos;

void main() {
    vec3 normalizedFragPos = outPos.xyz / outPos.w;
    vec2 UV = 0.5 * (normalizedFragPos.xy + vec2(1.0));

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

    // light info in world space
    vec3 lightDir = normalize(lightPos - fragPos);

	vec3 diffuseColor; vec3 specularColor;

    float ndotL = max(dot(normal, lightDir), 0.0);
    if (ndotL > 0.0) {
        // diffuse
		diffuseColor.x = float(diffuseSpec.x & 0xFF) / 255.0;
		diffuseColor.y = float(diffuseSpec.y & 0xFF) / 255.0;
		diffuseColor.z = float(diffuseSpec.z & 0xFF) / 255.0;
        vec3 diffuse = lightCol * diffuseColor * ndotL;

        // specular
        vec3 h = normalize(lightDir + eye);
        float spec = pow(max(dot(h, normal), 0.0), diffuseSpec.a);

		specularColor.x = float(uint(diffuseSpec.x) >> 8) / 255.0;
		specularColor.y = float(uint(diffuseSpec.y) >> 8) / 255.0;
		specularColor.z = float(uint(diffuseSpec.z) >> 8) / 255.0;
        vec3 specular = specularColor * spec;

        // attenuation
        float dist = length(lightPos - fragPos);
        float attenuation = lightLum / (1 + 1 * dist + 2 * dist * dist);

        outColor += diffuse * attenuation; // diffuse
        outColor += specular * attenuation; // specular
    }
    
    finalColor.rgb = outColor;
    finalColor.a   = 1.0;
}