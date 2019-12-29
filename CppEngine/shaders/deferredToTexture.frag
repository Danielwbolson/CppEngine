#version 450 core

layout (location = 0) out vec3 gNormal;
layout (location = 1) out uvec4 gDiffuseSpec;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D specularHighLightTex;
uniform sampler2D bumpTex;
uniform sampler2D normalTex;
uniform sampler2D dispTex;

uniform bool usingNormal;

uniform vec3 diffuse;
uniform vec3 specular;
uniform float specularExp;

in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;
in mat3 tbn;

// http://jcgt.org/published/0003/02/01/paper.pdf
// Octahedron encoding is a smarter compression algorithm that performs better than 
// R11xG11xB10 while using less bits. This compression will allow me use of 8 free bits
// in later work (ambient occlusion, stencil)
vec3 octahedronCompress(vec3);
vec2 signNotZero(vec2);
vec2 float32x3_to_oct(vec3);
vec3 oct_to_float8x3(vec2);

void main() {

	vec3 normal;
	if (usingNormal) {
		// https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		// Normal map
		vec3 n = texture(normalTex, fragUV).rgb * 2.0 - 1.0;
		normal = normalize(tbn * n);
	} else {
		normal = normalize(fragNorm);
	}
	gNormal = octahedronCompress(normal);


	// Pack our two 8bit diffuse/ambient and specular into 1 16bit value
	vec4 d = texture(diffuseTex, fragUV) * vec4(diffuse, 1);
	uint dx = uint(floor(255.0 * d.x));
	uint dy = uint(floor(255.0 * d.y));
	uint dz = uint(floor(255.0 * d.z));

	vec3 s = texture(specularTex, fragUV).rgb * specular;
	uint sx = uint(floor(255.0 * s.x));
	uint sy = uint(floor(255.0 * s.y));
	uint sz = uint(floor(255.0 * s.z));

	uint sE = uint(specularExp * texture(specularHighLightTex, fragUV).r);

	gDiffuseSpec = uvec4(dx | (sx << 8), dy | (sy << 8), dz | (sz << 8), sE);

}

// Compresses our normals via octahedron compression which allows us to shrink our bit usage
vec3 octahedronCompress(vec3 normal) {
	vec2 oct = float32x3_to_oct(normal);
	return oct_to_float8x3(oct);
}

// Returns +- 1
vec2 signNotZero(vec2 v) {
	return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

// Assume normalized input.  Output is on [-1, 1] for each component.
vec2 float32x3_to_oct(vec3 v) {
	//Project the sphere onto the octahedron, and then onto the xy plane
	vec2 projVec = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
	
	//Reflect the folds of the lower hemisphere over the diagonals
	return (v.z <= 0.0) ? ((1.0 - abs(projVec.yx)) * signNotZero(projVec)) : projVec;
}

// The caller should store the return value into a GL_RGB8 texture or attribute without modification.
vec3 oct_to_float8x3(vec2 v) {
	vec2 oct_2x12 = vec2(round(clamp(v, -1.0, 1.0) * 2047 + 2047));
	float t = floor(oct_2x12.y / 256.0);
	
	return floor(vec3(oct_2x12.x / 16.0, 
					  fract(oct_2x12.x / 16.0) * 256.0 + t, 
					  oct_2x12.y - t * 256.0)) / 255.0;
}