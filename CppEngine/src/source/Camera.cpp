
#include "Camera.h"
#include "SDL_Static_Helper.h"

Camera::Camera() {
    transform = new Transform();
}

Camera::~Camera() {
	delete transform;
}

Camera::Camera(const glm::vec3& lookat, const glm::vec3& u, 
    const int& w, const int& h, const float& f, const float& np, const float& fp) {

    lookAt = glm::vec3(lookat.x, lookat.y, lookat.z);
    up = glm::vec3(u.x, u.y, u.z);
    fov = f;
    near_plane = np;
    far_plane = fp;
    aspect_ratio = w / (float)h;

    proj = glm::perspective(fov, aspect_ratio, near_plane, far_plane);

    transform = new Transform();
}

void Camera::Update(const float& dt) {
    float forward = 2.0f * (SDL_Input::keyboard[SDL_SCANCODE_W] - SDL_Input::keyboard[SDL_SCANCODE_S]);
    float right = 2.0f * (SDL_Input::keyboard[SDL_SCANCODE_D] - SDL_Input::keyboard[SDL_SCANCODE_A]);
    transform->UpdateVelocity(forward, right);

    transform->rotation = glm::vec3(SDL_Input::xRel, SDL_Input::yRel, 0);

    view = glm::lookAt(transform->position, transform->position + transform->forward, up);

    transform->Update(dt);
}

Camera Camera::operator=(const Camera& c) {
    if (this == &c) return *this;

    this->lookAt = c.lookAt;
    this->up = c.up;
    this->fov = c.fov;
    this->aspect_ratio = c.aspect_ratio;
    this->near_plane = c.near_plane;
    this->far_plane = c.far_plane;

    return *this;
}