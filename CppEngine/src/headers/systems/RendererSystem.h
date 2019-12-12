
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
    GLuint positions;
    GLuint normals;
    GLuint diffuse;
    GLuint specular;
};

const float quadVerts[12] = {
    -0.5f, 0.5f,
    -0.5f, -0.5f,
	0.5f, -0.5f,

    0.5f, -0.5f,
	0.5f, 0.5f,
	-0.5f, 0.5f
};

struct MeshToDraw {
	Mesh* mesh;
	Material* material; 
	glm::mat4 model;
	GLuint vao;
	GLuint indexVbo;
	GLuint shaderProgram;
	glm::vec3 position;

	~MeshToDraw() {
		mesh = nullptr;
		material = nullptr;
	}
};

struct PointLightToDraw {
	glm::mat4 model; 
	glm::vec3 position;
	float luminance;
	glm::vec3 color;
	float radius;
};

struct PointLightToGPU {
	glm::vec4 position;
	glm::vec3 color;
	float luminance;
};

class RendererSystem : public Systems {
private:
    std::vector<ModelRenderer*> modelRenderers;
	std::vector<MeshToDraw> meshesToDraw;
	std::vector<MeshToDraw> transparentToDraw;
	std::vector<PointLightToGPU> pointLightsToGPU;

	// TODO: Need to add lights to the asset manager
    std::vector<PointLight*> pointLights;
	DirectionalLight* sun;
	std::vector<PointLightToDraw> pointLightsToDraw;

	GLubyte dummyData[4] = { 255, 255, 255, 255 };

    int screenWidth; int screenHeight;
    GBuffer gBuffer;
	GLuint lightVolumeShader;
	
	GLuint quadVAO; GLuint quadVBO;
	GLuint directionalLightShader;
	glm::mat4 lightProjView;

    Mesh* lightVolume;
	GLuint lightVolumeVAO; GLuint lightVolumeVBO; GLuint lightVolumeIBO;

	GLuint shadowMapShader;
	GLuint depthMapFBO; GLuint depthMap;
	const int shadowWidth = 4096; const int shadowHeight = 4096;

	glm::mat4 proj; glm::mat4 view;

	GLuint pointLightsSSBO = 0;

public:
	int totalTriangles = 0;

	RendererSystem(const int&, const int&);
    ~RendererSystem();

    void Setup();
    void Register(const Component*);

    void Update(const float&) {}
    void Render();

	void CullScene();
	void DrawShadows();
	void DeferredToTexture();
	void DeferredLighting();
	void DrawTransparent();
	void PostProcess();
	void DrawQuad();

	bool ShouldFrustumCull(const Mesh*, const glm::mat4&) const;
};

#endif
