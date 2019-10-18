
#ifndef SYSTEMS_H_
#define SYSTEMS_H_

#include "GameObject.h"
#include "Light.h"
#include "Component.h"

#include <vector>
#include <string>

class Systems {
protected:
    std::string componentType;
    std::vector<GameObject*> gameObjects;
    std::vector<Component*> components;

public:
	virtual ~Systems() {}

    virtual void Setup(const std::vector<GameObject*>&, const std::vector<Light*>&) = 0;
    virtual void ComponentType(const std::string&) const = 0;
    virtual void Register(const Component*) = 0;

    virtual void Update(const float&) = 0;
    virtual void Render() = 0;
};

#endif
