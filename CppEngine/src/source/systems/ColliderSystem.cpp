
#include "ColliderSystem.h"

ColliderSystem::ColliderSystem() {

}

ColliderSystem::~ColliderSystem() {
	for (int i = 0; i < colliders.size(); i++) {
		delete colliders[i];
	}
}

void ColliderSystem::Setup(const std::vector<GameObject*>& g, const std::vector<Light*>& l) {

}

void ColliderSystem::ComponentType(const std::string&) const {

}

void ColliderSystem::Register(const Component* c) {

}

void ColliderSystem::Update(const float&) {

}
