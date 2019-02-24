
#ifndef SCENE_H_
#define SCENE_H_

#include <SDL.h>

#include "Camera.h"
#include "GameObject.h"
#include "Material.h"
#include "Light.h"
#include "Collider.h"

#include <string>
#include <vector>

struct Light;

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