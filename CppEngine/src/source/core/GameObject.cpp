
#include "GameObject.h"
#include "Collider.h"
#include <algorithm>

GameObject::GameObject() {
	transform = MemoryManager::Allocate<Transform>();
	transform->gameObject = this;
	AddComponent(transform);
}

GameObject::~GameObject() {
	for (int i = 0; i < components.size(); i++) {
		MemoryManager::Free(components[i]);
	}
	components.clear();
}

GameObject::GameObject(const GameObject& rhs) {
    this->name = rhs.name;
    this->transform = rhs.transform->clone();

	for (int i = 0; i < components.size(); i++) {
		MemoryManager::Free(components[i]);
	}
    components = std::vector<Component*, MemoryAllocator<Component*> >();

    for (auto& element : rhs.components) {
        components.push_back(element->clone());
    }
}

GameObject& GameObject::operator=(const GameObject& rhs) {
    if (this == &rhs) return *this;
    this->name = rhs.name;
    this->transform = rhs.transform->clone();

	for (int i = 0; i < components.size(); i++) {
		MemoryManager::Free(components[i]);
	}
    components = std::vector<Component*, MemoryAllocator<Component*> >();
    for (auto& element : rhs.components) {
        components.push_back(element->clone());
    }

    return *this;
}

void GameObject::Update(const float& dt) {
    transform->Update(dt);

    for (int i = 0; i < (int)components.size(); i++) {
        components[i]->Update(dt);
    }
}

void GameObject::AddComponent(Component* c) {
    components.push_back(c);
    c->gameObject = this;
}

Component* GameObject::GetComponent(std::string componentType) {
	std::vector<Component*, MemoryAllocator<Component*> >::iterator it = std::find_if(components.begin(), components.end(), [&](const Component* c) {
        return c->ComponentType() == componentType;
    });

    if (it != components.end())
        return *it;
    else
        return nullptr;
}
