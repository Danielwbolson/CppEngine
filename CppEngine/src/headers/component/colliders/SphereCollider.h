
#ifndef SPHERE_COLLIDER_H_
#define SPHERE_COLLIDER_H_

#include "Collider.h"

#include "glm/glm.hpp"

class SphereCollider : public Collider {
    
private:
    float radius;

public:
    SphereCollider() {}
    SphereCollider(const glm::vec3&, const float&, const bool&);
    SphereCollider* clone() const;

    void Update(const float&);
    float MaxBoundsInDir(const glm::vec3&) const;
    bool CollisionDetect(const Collider&, const float&) const;
    float MaxBounds() const { return radius; }

    float Radius() const { return radius; }
    bool Dynamic() const { return dynamic; }
    glm::vec3 Position() const { return position; }

};

#endif
