
#include "Grab.h"
#include "Collider.h"
#include "InteractableObject.h"
#include "Transform.h"
#include "GameObject.h"

Grab::Grab() {
    componentType = "grab";
}

Grab::~Grab() {
	grabbedObj = nullptr;
}

Grab* Grab::clone() const {
    return new Grab(*this);
}

void Grab::Update(const float& dt) {
    Collider* c = (Collider*)gameObject->GetComponent("collider");
    if (c && c->colliding) {
        InteractableObject* io = (InteractableObject*)c->colliderObj->GetComponent("interactableObject");

        if (io && !isHolding) {
            GrabObject(io->gameObject);
            Collider* otherCol = (Collider*)io->gameObject->GetComponent("collider");
            otherCol->dynamic = true;
        }
    }

    if (grabbedObj && !grabbedObj->dead) {
        grabbedObj->transform->UpdateVelocity(
            gameObject->transform->velocity.x, gameObject->transform->velocity.z);

        grabbedObj->transform->position = gameObject->transform->position + 2.0f * gameObject->transform->forward;

        grabbedObj->transform->forward = gameObject->transform->forward;
    }
    else {
        isHolding = false;
        grabbedObj = nullptr;
    }
}

void Grab::GrabObject(GameObject* c) {
    isHolding = true;
    grabbedObj = c;

    grabbedObj->transform->position = gameObject->transform->position + 2.0f * gameObject->transform->forward;
    grabbedObj->transform->forward = gameObject->transform->forward;
}