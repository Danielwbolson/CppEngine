
#ifndef PHYSICS_SYSTEM_H_
#define PHYSICS_SYSTEM_H_

#include "Systems.h"

#include <vector>
#include <string>


class PhysicsSystem : public Systems {
private:


public:
    PhysicsSystem();

    void Setup(const std::vector<GameObject*>&, const std::vector<Light*>&);
    void ComponentType(const std::string&) const;
    void Register(const Component*);

    void Update(const float&);
    void Render() {}
};

#endif
