
#ifndef MESH_RENDERER_SYSTEM_H_
#define MESH_RENDERER_SYSTEM_H_

#include "Systems.h"

#include "glad/glad.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include "MeshRenderer.h"
#include "PointLight.h"
#include "Mesh.h"

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


class MeshRendererSystem : public Systems {
private:
    std::vector<MeshRenderer*> meshRenderers;
    std::vector<Light*> lights;
    std::vector<PointLight> pointLights;
    std::vector<glm::vec4> lightPositions;

    int screenWidth; int screenHeight;
    GBuffer gBuffer;
    GLuint lightVolume_Vao; GLuint lightVolume_Vbo; GLuint lightVolume_Ibo;
    GLuint quadVao; GLuint quadVbo;
    GLuint combinedShader;
    //GLuint lightUBO;

    Mesh lightSphere;

    int numLights;

public:
    MeshRendererSystem(const int&, const int&);
    ~MeshRendererSystem();

    void Setup(const std::vector<GameObject*>&, const std::vector<Light*>&);
    void ComponentType(const std::string&) const;
    void Register(const Component*);

    void Update(const float&) {}
    void Render() const;
};

#endif
