
#include "InteractableObject.h"

InteractableObject::InteractableObject() {
    componentType = "interactableObject";
}

InteractableObject* InteractableObject::clone() const {
    return new InteractableObject(*this);
}