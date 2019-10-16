
#ifndef INTERACTABLE_OBJECT_H_
#define INTERACTABLE_OBJECT_H_

#include "Component.h"

class InteractableObject : public Component {

private:

public:
    InteractableObject();
    InteractableObject* clone() const;

};

#endif
