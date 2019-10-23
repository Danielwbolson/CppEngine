
#ifndef COLLIDER_H_
#define COLLIDER_H_

#include "Component.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

class Collider : public Component {
public:
    glm::vec3 position;
    bool dynamic;
    bool colliding;
    GameObject* colliderObj;

	~Collider();

    virtual bool CollisionDetect(const Collider&, const float&) const = 0;
    virtual void Update(const float&) = 0;
    virtual float MaxBoundsInDir(const glm::vec3&) const = 0;
};

#endif