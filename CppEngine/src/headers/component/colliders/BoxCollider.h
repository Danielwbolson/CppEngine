
#ifndef BOX_COLLIDER_H_
#define BOX_COLLLIDER_H_

#include "Collider.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

class BoxCollider : public Collider {

private:
    float width, height;

public:
    BoxCollider() {}
    BoxCollider(const glm::vec3&, const float&, const float&, const bool&);
    BoxCollider* clone() const;

    BoxCollider(const BoxCollider&);
    BoxCollider operator=(const BoxCollider&);

    void Update(const float&);
    float MaxBoundsInDir(const glm::vec3&) const;
    bool CollisionDetect(const Collider&, const float&) const;

    float Width() const { return width; }
    float Height() const { return height; }
};

#endif
