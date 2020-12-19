
#ifndef RAY_TRACING_SYSTEM_H_
#define RAY_TRACING_SYSTEM_H_

#include "Systems.h"
#include "Globals.h"

#include "glad/glad.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include "RenderTypes.h"
#include "BVHTypes.h"

#include <vector>

class ModelRenderer;

class RayTracingSystem : public Systems {
private:
	std::vector<ModelRenderer*, MemoryAllocator<ModelRenderer*> > modelRenderers;
	std::vector<PointLightToGPU, MemoryAllocator<PointLightToGPU> > pointLightsToGPU;
	std::vector<PointLightIndicesSSBO, MemoryAllocator<PointLightIndicesSSBO> > pointLightIndicesSSBOToGPU;
	std::vector<PointLightIndicesUBO, MemoryAllocator<PointLightIndicesUBO> > pointLightIndicesUBOToGPU;
	std::vector<DirectionalLightToGPU, MemoryAllocator<DirectionalLightToGPU> > directionalLightsToGPU;

	glm::mat4 proj; glm::mat4 view;

	GLuint verticesSSBO; GLuint triangleSSBO;

	GLuint bakeLightsComputeShader; 
	GLuint rayTraceComputeShader;

	GLuint pointLightsUBO; 
	GLuint pointLightIndicesSSBO; GLuint pointLightIndicesUBO;
	GLuint bvhSSBO;
	GLuint triangleLightsSSBO; GLuint materialsUBO;

	GLuint finalQuadFBO; GLuint finalQuadRender; GLuint finalQuadDepth;
	GLuint finalQuadShader;
	// Quad vert info
	GLuint quadVAO; GLuint quadVBO;

	// Uniforms
	GLint uniDestTex;
	GLint uniCamPos;
	GLint uniProj;
	GLint uniView;
	GLint uniInvProj;
	GLint uniInvView;
	GLint uniNumPointLights;
	GLint uniDirectionalLightDir;
	GLint uniDirectionalLightCol;
	GLint uniMinBounds;
	GLint uniMaxBounds;

public:
	unsigned long long uboMemory = 0;
	unsigned long long ssboMemory = 0;

	// Timings
	GLuint timeQuery;
	long long bakeLightsTime;
	long long rayTraceTime;
	long long postFXTime;

	RayTracingSystem();
	~RayTracingSystem();

	void Setup();
	void Register(const Component*) {}

	void Update(const float&) {}
	void Render();

	void LightCull();
	void RayTrace();
	void PostProcess();
};

#endif //RAY_TRACING_SYSTEM_H_