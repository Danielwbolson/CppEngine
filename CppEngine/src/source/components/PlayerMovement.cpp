
#include "PlayerMovement.h"
#include "Transform.h"
#include "SDL_Static_Helper.h"
#include "GameObject.h"

PlayerMovement::PlayerMovement() {

}

PlayerMovement::PlayerMovement(const float& speed) {
    componentType = "playerMovement";
    this->speed = speed;
}

PlayerMovement* PlayerMovement::clone() const {
    return new PlayerMovement(*this);
}

PlayerMovement::PlayerMovement(const PlayerMovement& p) {
    this->speed = p.speed;
    this->gameObject = p.gameObject;
}

void PlayerMovement::Update(const float& dt) {
    forward = speed * (SDL_Input::keyboard[SDL_SCANCODE_W] - SDL_Input::keyboard[SDL_SCANCODE_S]);
    right = speed * (SDL_Input::keyboard[SDL_SCANCODE_D] - SDL_Input::keyboard[SDL_SCANCODE_A]);
    gameObject->transform->UpdateVelocity(forward, right);

    gameObject->transform->rotation = glm::vec3(SDL_Input::xRel, SDL_Input::yRel, 0);
}
