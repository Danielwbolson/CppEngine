#version 420 core

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecularExp;

uniform vec4 lightPos;
uniform vec4 lightCol;

uniform vec3 camPos;

in vec4 outPos;
out vec4 finalColor;

void main() {
    vec3 normalizedFragPos = outPos.xyz / outPos.w;
    vec2 UV = 0.5 * (normalizedFragPos.xy + vec2(1.0));

    vec3 fragPos      = texture(gPosition, UV).rgb;
    vec3 normal       = normalize(texture(gNormal, UV).rgb);
    vec3 diffuseColor = texture(gDiffuse, UV).rgb;
    vec4 specExp      = texture(gSpecularExp, UV);
    
    // eye
    vec3 eye = normalize(camPos-fragPos);
    vec3 outColor = vec3(0, 0, 0);

    // light info in world space
    vec3 lightDir = normalize(lightPos.xyz - fragPos);

    float ndotL = max(dot(normal, lightDir), 0.0);
    if (ndotL > 0.0) {
        // diffuse
        vec3 diffuse = lightCol.rgb * diffuseColor * ndotL;

        // specular
        vec3 h = normalize(lightDir + eye);
        float spec = pow(max(dot(h, normal), 0.0), specExp.a);
        vec3 specular = specExp.rgb * spec;

        // attenuation
        float dist = length(lightPos.xyz - fragPos);
        float attenuation = 1.0 / (1 + 5 * dist + dist * dist);

        outColor += diffuse * attenuation; // diffuse
        outColor += specular * attenuation; // specular
    }
    
    finalColor.rgb = outColor;
    finalColor.a   = 1.0;
}