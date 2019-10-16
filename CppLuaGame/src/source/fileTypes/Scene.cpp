
#include "Scene.h"
#include "Collider.h"

#include <algorithm>

Scene::Scene() : window_width(640), window_height(480), background{ glm::vec3(0, 0, 0) } {}

Scene::~Scene() {
    gameObjects.clear();

    instances.clear();

    lights.clear();
}

GameObject* Scene::FindGameObject(const std::string& name) {
    std::vector<GameObject*>::iterator it = std::find_if(gameObjects.begin(), gameObjects.end(), [&](const GameObject* c) {
        return c->name == name;
    });

    if (it != gameObjects.end())
        return *it;
    else
        return nullptr;
}

GameObject* Scene::FindInstance(const std::string& name) {
    std::vector<GameObject*>::iterator it = std::find_if(instances.begin(), instances.end(), [&](const GameObject* c) {
        return c->name == name;
    });

    if (it != instances.end())
        return *it;
    else
        return nullptr;
}

void Scene::Update(const float dt) {
    for (int i = 0; i < instances.size(); i++) {
        if (instances[i]->dead) {
            delete instances[i];
            instances[i] = NULL;
            instances.erase(instances.begin() + (i--));
        }
        instances[i]->Update(dt);
    }
}

void Scene::CollisionChecks(const float& dt) const {
    for (int i = 0; i < instances.size(); i++) {
        bool b = false;
        Collider* c = (Collider*)instances[i]->GetComponent("collider");
        if (c && c->dynamic) {
            c->colliderObj = nullptr;
            c->colliding = false;

            for (int j = 0; j < instances.size(); j++) {
                Collider* other = (Collider*)instances[j]->GetComponent("collider");
                if (other && i != j && !other->dynamic) {
                    other->colliderObj = nullptr;
                    other->colliding = false;

                    if (c->CollisionDetect(*other, dt)) {
                        c->gameObject->transform->velocity = glm::vec3(0, 0, 0);
                        c->colliding = true;
                        other->colliding = true;
                        c->colliderObj = other->gameObject;
                        other->colliderObj = c->gameObject;

                        b = true;
                        break;
                    }
                }
            }
            if (b)
                break;
        }
    }
}
