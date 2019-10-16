
#ifndef GAMEOBJECT_H_
#define GAMEOBJECT_H_

#include "Component.h"
#include "Transform.h"

#include <vector>
#include <string>

class GameObject {

public:
	Transform* transform;

	std::vector<Component*> components;
	std::string name;
	bool dead = false;

	GameObject();
	~GameObject();
	GameObject(const GameObject&);
	GameObject& operator=(const GameObject&);

	void Update(const float&);

	void AddComponent(Component*);
	Component* GetComponent(const std::string);
};
#endif 
