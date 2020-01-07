
#ifndef PHYSICS_SYSTEM_H_
#define PHYSICS_SYSTEM_H_

#include "Systems.h"

#include <vector>
#include <string>

// Find a library like bullet so that I can focus on rendering
class PhysicsSystem : public Systems {
private:


public:
    PhysicsSystem();
	~PhysicsSystem();

    void Setup();
    void Register(const Component*);

    void Update(const float&);
    void Render() {}
};

#endif
