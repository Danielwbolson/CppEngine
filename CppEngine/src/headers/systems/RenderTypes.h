
#ifndef RAY_TRACING_SYSTEM_TYPES_H_
#define RAY_TRACING_SYSTEM_TYPES_H_

#include "GlobalMacros.h"

#include "glad/glad.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#define MAX_LIGHTS_PER_BLOCK 7

const float quadVerts[12] = {
	-1.0f, 1.0f,
	-1.0f, -1.0f,
	1.0f, -1.0f,

	1.0f, -1.0f,
	1.0f, 1.0f,
	-1.0f, 1.0f
};

/***** * * * * * GPU * * * * * *****/

#pragma pack(push, 1)
struct LightTile {
	unsigned int pointLightIndices[1023]; // 1023 point lights supported
	unsigned int numLights;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(LightTile, 4);
ASSERT_STRUCT_UP_TO_DATE(LightTile, 1024 * 4);


#pragma pack(push, 1)
struct PointLightToGPU {
	glm::vec4 position_and_radius;
	glm::vec4 color_and_luminance;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(PointLightToGPU, 16);
ASSERT_STRUCT_UP_TO_DATE(PointLightToGPU, 32);


#pragma pack(push, 1)
struct PointLightIndicesToGPU {
	uint32_t indices[MAX_LIGHTS_PER_BLOCK];
	uint32_t numLights;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(PointLightIndicesToGPU, 16);
ASSERT_STRUCT_UP_TO_DATE(PointLightIndicesToGPU, 32);


#pragma pack(push, 1)
struct DirectionalLightToGPU {
	glm::vec4 direction;
	glm::vec4 color_and_luminance;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(DirectionalLightToGPU, 16);
ASSERT_STRUCT_UP_TO_DATE(DirectionalLightToGPU, 32);


#pragma pack(push, 1)
struct GPUMaterial {
	uint64_t diffuseTexture;
	uint64_t normalTexture;

	uint64_t specularTexture;
	uint64_t alphaTexture;

	uint32_t diffuse; // 8/8/8/NA
	uint32_t specular; // 8/8/8/NA
	float specularExponent;
	uint32_t usingNormal_Specular_Alpha; // 8/8/8/0

	uint64_t pad;
	uint64_t pad1;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(GPUMaterial, 8);
ASSERT_STRUCT_UP_TO_DATE(GPUMaterial, 64);


#pragma pack(push, 1)
struct GPUVertex {
	glm::vec4 position_and_u;
	glm::vec4 normal_and_v;
	glm::vec4 tangent;
	glm::vec4 bitangent;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(GPUVertex, 16);
ASSERT_STRUCT_UP_TO_DATE(GPUVertex, 64);


#pragma pack(push, 1)
struct GPUTriangle {
	uint32_t indices[3];
	uint32_t materialIndex;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(GPUTriangle, 16);
ASSERT_STRUCT_UP_TO_DATE(GPUTriangle, 16);



/****** * * * * * CPU * * * * * *****/

struct MeshToDraw {
	class Mesh* mesh;
	class Material* material;
	glm::mat4 model;
	GLuint vao;
	GLuint shaderProgram;
	glm::vec3 position;

	~MeshToDraw() {
		mesh = nullptr;
		material = nullptr;
	}
};


#endif