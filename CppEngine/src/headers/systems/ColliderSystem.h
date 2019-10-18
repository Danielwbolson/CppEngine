
#ifndef COLLIDER_SYSTEM_H_
#define COLLIDER_SYSTEM_H_

#include "Systems.h"
#include "Collider.h"

#include <string>

class ColliderSystem : public Systems {
private:
    std::vector<Collider*> colliders;

public:
    ColliderSystem();
	~ColliderSystem();

    void Setup(const std::vector<GameObject*>&, const std::vector<Light*>&);
    void ComponentType(const std::string&) const;
    void Register(const Component*);

    void Update(const float&);
    void Render() {}
};

#endif
