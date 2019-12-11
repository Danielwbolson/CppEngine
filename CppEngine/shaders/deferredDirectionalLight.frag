#version 450 core

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecularExp;

uniform vec3 lightDir;
uniform vec3 lightCol;

uniform vec3 camPos;

in vec2 UV;
out vec4 finalColor;

void main() {

    vec3 fragPos      = texture(gPosition, UV).rgb;
    vec3 normal       = normalize(texture(gNormal, UV).rgb);
    vec3 diffuseColor = texture(gDiffuse, UV).rgb;
    vec4 specExp      = texture(gSpecularExp, UV);
    
    // eye
    vec3 eye = normalize(camPos-fragPos);
    vec3 outColor = vec3(0, 0, 0);

    float ndotL = max(dot(normal, -lightDir), 0.0);
    if (ndotL > 0.0) {
        // diffuse
        vec3 diffuse = lightCol * diffuseColor * ndotL;

        // specular
        vec3 h = normalize(-lightDir + eye);
        float spec = pow(max(dot(h, normal), 0.0), specExp.a);
        vec3 specular = specExp.rgb * spec;

        outColor += diffuse; // diffuse
        outColor += specular; // specular
    }
    
    finalColor.rgb = outColor;
    finalColor.a   = 1.0;
}