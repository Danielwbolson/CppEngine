
#ifndef MODEL_RENDERER_SYSTEM_H_
#define MODEL_RENDERER_SYSTEM_H_

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


class ModelRendererSystem : public Systems {
private:
    std::vector<ModelRenderer*> modelRenderers;
    std::vector<PointLight> pointLights;
    std::vector<glm::vec4> lightPositions;

	GLubyte dummyData[4] = { 255, 255, 255, 255 };

    int screenWidth; int screenHeight;
    GBuffer gBuffer;
    GLuint lightVolume_Vao; GLuint lightVolume_Vbo; GLuint lightVolume_Ibo;
    GLuint quadVao; GLuint quadVbo;
    GLuint combinedShader;

    Mesh* lightSphere;

public:
	int totalTriangles = 0;

    ModelRendererSystem(const int&, const int&);
    ~ModelRendererSystem();

    void Setup();
    void Register(const Component*);

    void Update(const float&) {}
    void Render();

	void GenTexture(GLuint* id, const int& texIndex, Texture* tex);
	void GenNullTexture(GLuint* id, const int& texIndex);
	bool FrustumCull(const Mesh*, const glm::mat4&, const glm::mat4&) const;
};

#endif
