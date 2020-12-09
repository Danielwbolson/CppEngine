
#ifndef RENDERER_SYSTEM_H_
#define RENDERER_SYSTEM_H_

#include "Systems.h"
#include "Globals.h"
#include "RenderTypes.h"

class ModelRenderer;
class Component;
class Mesh;

#include <vector>

struct GBuffer {
	GLuint id;
	GLuint normals;
	GLuint diffuseSpec;
	GLuint depth;
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
	GLuint pointLightsSSBO;
	GLuint lightTilesSSBO; // Max of 512 lights per tile

	// Shadows
	GLuint shadowMapShader;
	GLuint depthMapFBO; GLuint depthMap;
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
