
#ifndef RAY_TRACING_SYSTEM_H_
#define RAY_TRACING_SYSTEM_H_

#include "Systems.h"
#include "Globals.h"

#include "glad/glad.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include "RenderTypes.h"

#include <vector>


class ModelRenderer;

class RayTracingSystem : public Systems {
private:
	std::vector<ModelRenderer*, MemoryAllocator<ModelRenderer*> > modelRenderers;
	std::vector<PointLightToGPU, MemoryAllocator<PointLightToGPU> > pointLightsToGPU;
	std::vector<DirectionalLightToGPU, MemoryAllocator<DirectionalLightToGPU> > directionalLightsToGPU;

	glm::mat4 proj; glm::mat4 view;

	GLuint worldVAO; GLuint verticesVBO; GLuint triangleSSBO;

	GLuint bakeLightsComputeShader; 
	GLuint rayTraceComputeShader;

	GLuint pointLightsUBO; GLuint bvhSSBO; 
	GLuint triangleLightsSSBO; GLuint materialsSSBO;

	GLuint finalQuadFBO; GLuint finalQuadRender; GLuint finalQuadDepth;
	GLuint finalQuadShader;
	// Quad vert info
	GLuint quadVAO; GLuint quadVBO;

	// Uniforms
	GLint uniDestTex;
	GLint uniCamPos;
	GLint uniProjView;
	GLint uniView;
	GLint uniInvProj;
	GLint uniInvView;
	GLint uniNumPointLights;

public:

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