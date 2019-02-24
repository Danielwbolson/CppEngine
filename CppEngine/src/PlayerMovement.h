
#ifndef PLAYER_MOVEMENT_H_
#define PLAYER_MOVEMENT_H_

#include "Component.h"
#include "Transform.h"
#include "SDL_Static_Helper.h"
#include "Globals.h"

class PlayerMovement : public Component {

private:
    float speed;
    float forward, right;

public:
    PlayerMovement();
    PlayerMovement(const float&);
    PlayerMovement* clone() const;

    PlayerMovement(const PlayerMovement&);

    void Update(const float&);
};

#endif
