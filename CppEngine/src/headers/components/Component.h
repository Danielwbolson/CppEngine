
#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <string>
#include "Globals.h"

class GameObject;

class Component {

protected:
    std::string componentType;

public:
    GameObject* gameObject;
    std::string name;

    Component() {}
	~Component();

    // Returns camelCase name of components
    std::string ComponentType() const { return componentType; }

    virtual void Update(const float&) {}
    virtual Component* clone() const = 0;

};
#endif 