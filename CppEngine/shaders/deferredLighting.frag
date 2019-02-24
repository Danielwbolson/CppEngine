#version 420 core

struct PointLight {
    vec4 color;
    vec4 position;
};

#define NUM_POINT_LIGHTS 20
layout (std140) uniform pointLightBlock {
    PointLight pointLights[NUM_POINT_LIGHTS];
};

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecularExp;

uniform mat4 view;
uniform vec4 lightPos;

in vec2 UV;
in vec4 outPos;

out vec4 finalColor;

void main() {
    vec3 normalizedFragPos = outPos.xyz / outPos.w;
    vec2 UV = 0.5 * (normalizedFragPos.xy + vec2(1.0));

    vec3 fragPos      = texture(gPosition, UV).rgb;
    vec3 n            = normalize(texture(gNormal, UV).rgb);
    vec3 diffuseColor = texture(gDiffuse, UV).rgb;
    vec4 specExp      = texture(gSpecularExp, UV);
    
    // eye
    vec3 eye = normalize(-fragPos);
    vec3 outColor = vec3(0, 0, 0);
    
    // point lights
    for (int i = 0; i < NUM_POINT_LIGHTS; i++) {

        // light info in view space
        vec3 viewLightPos = (view * pointLights[i].position).xyz;
        vec3 lightDir = normalize(viewLightPos - fragPos);

        float ndotL = max(dot(n, lightDir), 0.0);
        if (ndotL > 0.0) {
            // diffuse
            vec3 diffuse = pointLights[i].color.rgb * diffuseColor * ndotL;

            // specular
            vec3 h = normalize(lightDir + eye);
            float spec = pow(max(dot(h, n), 0.0), specExp.a);
            vec3 specular = specExp.rgb * spec;

            // attenuation
            float dist = length(viewLightPos - fragPos);
            float attenuation = 1.0 / (dist * dist);

            outColor += diffuse * attenuation; // diffuse
            outColor += specular * attenuation; // specular
        }
    }
    
    finalColor.rgb = outColor;
    finalColor.a   = 1.0;
}