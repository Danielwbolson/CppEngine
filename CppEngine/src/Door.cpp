
#include "Door.h"

Door::Door(const std::string& k) {
    password = k;
    componentType = "door";
}

Door* Door::clone() const {
    return new Door(*this);
}

void Door::Update(const float& dt) {
    Collider* c = (Collider*)gameObject->GetComponent("collider");
    if (c && c->colliding) {
        Key* k = (Key*)c->colliderObj->GetComponent("key");

        if (k && password == k->Password()) {
            gameObject->dead = true;
            c->colliderObj->dead = true;
        }
    }
}