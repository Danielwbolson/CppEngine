
#ifndef RENDERER_SYSTEM_H_
#define RENDERER_SYSTEM_H_

#include "Systems.h"

#include "glad/glad.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include "GameObject.h"
#include "ModelRenderer.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "Model.h"
#include "Mesh.h"
#include "Texture.h"

#include <vector>

struct GBuffer {
	GLuint id;
	GLuint normals;
	GLuint diffuseSpec;
	GLuint depth;
};

const float quadVerts[12] = {
	-1.0f, 1.0f,
	-1.0f, -1.0f,
	1.0f, -1.0f,

	1.0f, -1.0f,
	1.0f, 1.0f,
	-1.0f, 1.0f
};

struct MeshToDraw {
	Mesh* mesh;
	Material* material; 
	glm::mat4 model;
	GLuint vao;
	GLuint shaderProgram;
	glm::vec3 position;

	~MeshToDraw() {
		mesh = nullptr;
		material = nullptr;
	}
};

struct PointLightToGPU {
	glm::vec4 position_and_radius;
	glm::vec4 color_and_luminance;
};
struct DirectionalLightToGPU {
	glm::vec4 direction;
	glm::vec3 color;
	float luminance;
};

class RendererSystem : public Systems {
private:
	std::vector<ModelRenderer*, MemoryAllocator<ModelRenderer*> > modelRenderers;
	std::vector<MeshToDraw, MemoryAllocator<MeshToDraw> > meshesToDraw;
	std::vector<MeshToDraw, MemoryAllocator<MeshToDraw> > transparentToDraw;

	std::vector<PointLightToGPU, MemoryAllocator<PointLightToGPU> > pointLightsToGPU;
	std::vector<DirectionalLightToGPU, MemoryAllocator<DirectionalLightToGPU> > directionalLightsToGPU;

	GLubyte dummyData[4] = { 255, 255, 255, 255 };

	glm::mat4 proj; glm::mat4 view;

	// Deferred
	GBuffer gBuffer;

	// Quad vert info
	GLuint quadVAO; GLuint quadVBO;
	
	// Directional Light quad
	GLuint directionalLightShader;
	glm::mat4 lightProjView;

	// Tiled lighting variables
	GLuint tiledComputeShader;
	GLuint tiledPointLightsSSBO;
	GLuint lightTilesSSBO; // Max of 512 lights per tile

	// Shadows
	GLuint shadowMapShader;
	GLuint depthMapFBO; GLuint depthMap;
	GLuint pointLightsSSBO = 0;
	const int shadowWidth = 4096; const int shadowHeight = 4096;

	// Post processing
	GLuint finalQuadFBO; GLuint finalQuadRender;
	GLuint finalQuadShader;

public:
	int totalTriangles = 0;

	// Timings
	GLuint timeQuery;
	long long depthPrePassTime = 0;
	long long tileComputeTime;
	long long cullTime; long long shadowTime;
	long long deferredToTexTime; long long deferredLightsTime;
	long long transparentTime; long long postFXXTime;

	RendererSystem();
	~RendererSystem();

	void Setup();
	void Register(const Component*);

	void Update(const float&) {}
	void Render();

	void OpaqueDepthPrePass();
	void CullScene();
	void DrawShadows();
	void DeferredToTexture();
	void TiledCompute();
	void DeferredLighting();
	void DrawTransparent();
	void PostProcess();

	bool ShouldFrustumCull(const Mesh*, const glm::mat4&) const;
};

#endif
