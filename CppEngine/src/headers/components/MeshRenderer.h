
#ifndef MESH_RENDERER_H_
#define MESH_RENDERER_H_

#include "glad/glad.h"

#include "Component.h"
#include "Mesh.h"
#include "Material.h"

class MeshRenderer : public Component {

public:
    // 0 : position, 1 : normals, 2 : uvs, 3 : indices
    GLuint vbo[4];
    GLuint vao;

    Mesh* mesh;
    Material* material;

public:
    MeshRenderer() {}
	~MeshRenderer();

    MeshRenderer(Mesh*, Material*);
    MeshRenderer* clone() const;

};
#endif
