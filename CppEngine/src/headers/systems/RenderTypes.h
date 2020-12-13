
#ifndef RAY_TRACING_SYSTEM_TYPES_H_
#define RAY_TRACING_SYSTEM_TYPES_H_

#include "GlobalMacros.h"

#include "glad/glad.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

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
struct LightsPerTriangle {
public:
	uint8_t lightIndices[255]; // 255 bytes
	uint8_t numAffectingLights; // 1 byte
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(LightsPerTriangle);
ASSERT_STRUCT_UP_TO_DATE(LightsPerTriangle, 256);


#pragma pack(push, 1)
struct LightTile {
	unsigned int pointLightIndices[1023]; // 1023 point lights supported
	unsigned int numLights;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(LightTile);
ASSERT_STRUCT_UP_TO_DATE(LightTile, 1024 * 4);


#pragma pack(push, 1)
struct PointLightToGPU {
	glm::vec4 position_and_radius;
	glm::vec4 color_and_luminance;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(PointLightToGPU);
ASSERT_STRUCT_UP_TO_DATE(PointLightToGPU, 32);


#pragma pack(push, 1)
struct DirectionalLightToGPU {
	glm::vec4 direction;
	glm::vec4 color_and_luminance;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(DirectionalLightToGPU);
ASSERT_STRUCT_UP_TO_DATE(DirectionalLightToGPU, 32);


#pragma pack(push, 1)
struct GPUMaterial {
	uint64_t diffuseTexture;
	uint64_t specularTexture;
	uint64_t specularHighlightTexture;
	uint64_t normalTexture;
	uint64_t alphaTexture;
	uint64_t pad[3];

	//glm::u8vec4 diffuse;
	//glm::u8vec4 specular;
	//glm::u8vec4 transmissive;
	//float specularExponent;
	//float indexOfRefraction;
	//uint8_t usingNormal;
	//uint8_t pad[3];
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(GPUMaterial);
ASSERT_STRUCT_UP_TO_DATE(GPUMaterial, 64);


#pragma pack(push, 1)
struct GPUVertex {
	glm::vec4 position;
	glm::vec4 normal;
	glm::vec4 uv;
	glm::vec4 tangents;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(GPUVertex);
ASSERT_STRUCT_UP_TO_DATE(GPUVertex, 64);


#pragma pack(push, 1)
struct GPUTriangle {
	uint32_t indices[3];
	uint32_t materialIndex;
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(GPUTriangle);
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