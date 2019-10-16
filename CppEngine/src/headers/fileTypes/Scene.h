
#ifndef SCENE_H_
#define SCENE_H_

#include "GameObject.h"
#include "Light.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include <string>
#include <vector>

class Scene {
public:
    int window_width;
    int window_height;
    int cube_width = 2;

    std::vector<GameObject*> gameObjects;
    std::vector<GameObject*> instances;
    std::vector<Light*> lights;
    glm::vec3 background;

    Scene();
    ~Scene();
    GameObject* FindGameObject(const std::string&);
    GameObject* FindInstance(const std::string&);

    void Update(const float dt);
    void CollisionChecks(const float&) const;
};

#endif