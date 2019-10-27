
#include "ColliderSystem.h"

ColliderSystem::ColliderSystem() {

}

ColliderSystem::~ColliderSystem() {
	for (int i = 0; i < colliders.size(); i++) {
		delete colliders[i];
	}
}

void ColliderSystem::Setup() {

}

void ColliderSystem::Register(const Component* c) {

}

void ColliderSystem::Update(const float&) {

}
