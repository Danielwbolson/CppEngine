#version 420 core

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecularExp;

uniform vec4 lightPos;
uniform vec4 lightCol;

uniform mat4 view;

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
	vec3 camPos = vec3(view[0][3], view[1][3], view[2][3]);
    vec3 eye = normalize(camPos-fragPos);
    vec3 outColor = diffuseColor * 0.001f; //vec3(0, 0, 0);

    // light info in view space
    vec3 viewLightPos = (view * lightPos).xyz;
    vec3 lightDir = normalize(viewLightPos - fragPos);

    float ndotL = max(dot(n, lightDir), 0.0);
    if (ndotL > 0.0) {
        // diffuse
        vec3 diffuse = lightCol.rgb * diffuseColor * ndotL;

        // specular
        vec3 h = normalize(lightDir + eye);
        float spec = pow(max(dot(h, n), 0.0), specExp.a);
        vec3 specular = specExp.rgb * spec;

        // attenuation
        float dist = length(viewLightPos - fragPos);
        float attenuation = 1.0 / (10 * dist * dist);

        outColor += diffuse * attenuation; // diffuse
        outColor += specular * attenuation; // specular
    }
    
    finalColor.rgb = outColor;
    finalColor.a   = 1.0;
}