#version 450 core

in vec3 inPos;
in vec3 inNorm;
in vec2 inUV;
in vec3 inTan;
in vec3 inBitan;

uniform bool usingNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 outPos;

out vec3 fragNorm;
out vec3 fragPos;
out vec2 fragUV;
out mat3 tbn;

void main() {
	gl_Position = proj * view * model * vec4(inPos,1.0);
	fragPos = (model * vec4(inPos,1.0)).xyz;
	fragNorm = normalize(vec3(transpose(inverse(model)) * vec4(inNorm, 0.0)));

	if (usingNormal) {
		// https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		vec3 fragBitan = normalize(vec3(model * vec4(inBitan, 0.0)).xyz);
		vec3 fragTan = normalize(vec3(model * vec4(inTan, 0.0)).xyz);
		tbn = mat3(fragTan, fragBitan, fragNorm);
	}

	fragUV = inUV;
	outPos = gl_Position;
}