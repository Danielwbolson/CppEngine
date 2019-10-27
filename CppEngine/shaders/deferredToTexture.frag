#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuse;
layout (location = 3) out vec4 gSpecularExp;

uniform sampler2D ambientTex;
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D specularHighLightTex;
uniform sampler2D bumpTex;
uniform sampler2D dispTex;
uniform sampler2D alphaTex;

uniform bool usingAmbientTex;
uniform bool usingDiffuseTex;
uniform bool usingSpecularTex;
uniform bool usingSpecularHighLightTex;
uniform bool usingBumpTex;
uniform bool usingDispTex;
uniform bool usingAlphaTex;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float specularExp;
uniform float opacity;

in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;

void main() {

    gPosition = fragPos;
    gNormal = normalize(fragNorm);

	vec4 a = vec4(ambient, 1);
	if (usingAmbientTex) { a = a * texture(ambientTex, fragUV); }
	vec4 d = vec4(diffuse, 1);
	if (usingDiffuseTex) { d = d * texture(diffuseTex, fragUV); }
	float o = opacity;
	if (usingAlphaTex) { o = o * texture(alphaTex, fragUV).r; }

	gDiffuse = vec4(a.xyz + d.xyz, o);

	vec4 s = vec4(specular, 1);
	if (usingSpecularTex) { s = s * texture(specularTex, fragUV); }
	float sE = specularExp;
	if (usingSpecularHighLightTex) { sE = sE * texture(specularHighLightTex, fragUV).r; }

    gSpecularExp = vec4(s.xyz, sE);

}