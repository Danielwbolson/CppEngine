
#ifndef COLLIDER_SYSTEM_H_
#define COLLIDER_SYSTEM_H_

#include "MemoryAllocator.h"
#include "Systems.h"
#include "Collider.h"

#include <string>

class ColliderSystem : public Systems {
private:
    std::vector<Collider*, MemoryAllocator<Collider*> > colliders;

public:
    ColliderSystem();
	~ColliderSystem();

    void Setup();
    void Register(const Component*);

    void Update(const float&);
    void Render() {}
};

#endif
