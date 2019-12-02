#version 410 core

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

uniform vec4 lightPos;
uniform vec4 lightCol;
uniform vec3 camPos;

in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;
in mat3 tbn;

out vec4 finalColor;

void main() {

	// calculate normal
	vec3 n;
	if (usingBump) {
	float diff = texture(bumpTex, fragUV).r - 0.5;
		n = normalize(fragNorm + fragNorm * diff);
	} else if (usingNormal) {
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
	float o = texture(alphaTex, fragUV).r;
	vec3 diffuseColor = vec3(a.xyz + d.xyz);

	// specular
	vec3 spec = texture(specularTex, fragUV).rgb * specular;
	float specExp = texture(specularHighLightTex, fragUV).r * specularExp;

	// eye
    vec3 eye = normalize(camPos-fragPos);
    vec3 outColor = vec3(0, 0, 0);

    // light info in world space
    vec3 lightDir = normalize(lightPos.xyz - fragPos);

    float ndotL = max(dot(n, lightDir), 0.0);
    if (ndotL > 0.0) {
        // diffuse
        diffuseColor = lightCol.rgb * diffuseColor * ndotL;

        // specular
        vec3 h = normalize(lightDir + eye);
        float exponent = pow(max(dot(h, n), 0.0), specExp);
        vec3 specularColor = spec * exponent;

        // attenuation
        float dist = length(lightPos.xyz - fragPos);
        float attenuation = 1.0 / (1 + 5 * dist + dist * dist);

        outColor += diffuseColor * attenuation; // diffuse
        outColor += specularColor * attenuation; // specular
    }

    //finalColor.rgb = outColor;
	//finalColor.a = o;

	finalColor = vec4(diffuseColor, o);

}