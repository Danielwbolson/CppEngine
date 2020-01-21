
#ifndef SCENE_H_
#define SCENE_H_

#include "GameObject.h"

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include <string>
#include <vector>

class Scene {
public:
    int windowWidth;
    int windowHeight;

    std::vector<GameObject*, MemoryAllocator<GameObject*> > gameObjects;
    std::vector<GameObject*, MemoryAllocator<GameObject*> > instances;

    std::vector<PointLight, MemoryAllocator<PointLight> > pointLights;
    std::vector<DirectionalLight, MemoryAllocator<DirectionalLight> > directionalLights;
    std::vector<SpotLight, MemoryAllocator<SpotLight> > spotLights;

    glm::vec3 background;

    Scene();
    ~Scene();
    GameObject* FindGameObject(const std::string&);
    GameObject* FindInstance(const std::string&);

    void Update(const float dt);
    void CollisionChecks(const float&) const;
};

#endif