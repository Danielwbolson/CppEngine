
#ifndef GRAB_H_
#define GRAB_H_

#include "Component.h"

class Grab : public Component {

private:
    bool isHolding;
    GameObject* grabbedObj;

public:
    Grab();
    Grab* clone() const;

    void GrabObject(GameObject*);
    void Update(const float&);

};

#endif
