
#include "SphereCollider.h"
#include "GameObject.h"

SphereCollider::SphereCollider(const glm::vec3& p, const float& r, const bool& d) {
    componentType = "collider";

    position = p;
    radius = r;
    dynamic = d;
}

SphereCollider* SphereCollider::clone() const {
	return memoryManager->Allocate<SphereCollider>(*this);
}

void SphereCollider::Update(const float& dt) {
    glm::vec3 p = gameObject->transform->position;
    position = glm::vec3(p.x, p.y, p.z);
}

float SphereCollider::MaxBoundsInDir(const glm::vec3& v) const {
    return radius;
}

bool SphereCollider::CollisionDetect(const Collider& c, const float& dt) const {
    Transform* t = gameObject->transform;
    glm::vec3 v = t->velocity * dt;

    glm::vec3 vec = c.position - (position + 3.0f * glm::vec3(v.x, v.y, v.z));
    float minDist = MaxBoundsInDir(glm::normalize(vec)) + c.MaxBoundsInDir(-glm::normalize(vec));
    float diff = minDist - glm::length(vec);

    return (diff >= 0.01f);
}