
#ifndef RENDERER_SYSTEM_H_
#define RENDERER_SYSTEM_H_

#include "Systems.h"

#include "glad/glad.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include "GameObject.h"
#include "ModelRenderer.h"
#include "PointLight.h"
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

const GLfloat quadVerts[12] = {
    -1, 1,
    -1, -1,
    1, -1,

    -1, 1,
    1, -1,
    1, 1
};

struct MeshToDraw {
	Mesh* mesh;
	Material* material; 
	glm::mat4 model;
	GLuint vao;
	GLuint indexVbo;
	GLuint shaderProgram;

	~MeshToDraw() {
		mesh = nullptr;
		material = nullptr;
	}
};

struct PointLightToDraw {
	float luminance;
	float radius;
	glm::vec4 position;
	glm::vec4 color;
	glm::mat4 model;

};


class RendererSystem : public Systems {
private:
    std::vector<ModelRenderer*> modelRenderers;
	std::vector<MeshToDraw> meshesToDraw;
	std::vector<MeshToDraw> transparentToDraw;

	// TODO: Need to add lights to the asset manager
    std::vector<PointLight*> pointLights;
	std::vector<PointLightToDraw> pointLightsToDraw;

	GLubyte dummyData[4] = { 255, 255, 255, 255 };

    int screenWidth; int screenHeight;
    GBuffer gBuffer;
    GLuint lightVolume_Vao; GLuint lightVolume_Vbo; GLuint lightVolume_Ibo;
    GLuint combinedShader;

    Mesh* lightVolume;

public:
	int totalTriangles = 0;

	RendererSystem(const int&, const int&);
    ~RendererSystem();

    void Setup();
    void Register(const Component*);

    void Update(const float&) {}

    void Render();

	void DeferredPass(const glm::mat4&, const glm::mat4&, const glm::mat4&);
	void ForwardPass(const glm::mat4&, const glm::mat4&, const glm::mat4&);

	void DrawShadows();
	void PostProcess();

	bool ShouldFrustumCull(const Mesh*, const glm::mat4&, const glm::mat4&) const;
};

#endif
