
#include "Camera.h"
#include "SDL_Static_Helper.h"

Camera::Camera() {
    transform = new Transform();
}

Camera::~Camera() {
	delete transform;
}

Camera::Camera(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& u, 
    const int& w, const int& h, const float& f, const float& np, const float& fp) {

	transform = new Transform();
	transform->position = pos;
	transform->forward = glm::normalize(dir);
	transform->up = glm::normalize(u);

    fov = f;
    near_plane = np;
    far_plane = fp;
    aspect_ratio = w / (float)h;

    proj = glm::perspective(fov, aspect_ratio, near_plane, far_plane);
	view = glm::lookAt(transform->position, transform->position + transform->forward, transform->up);
}

void Camera::Update(const float& dt) {
    float forward = 4.0f * (SDL_Input::keyboard[SDL_SCANCODE_W] - SDL_Input::keyboard[SDL_SCANCODE_S]);
    float right = 4.0f * (SDL_Input::keyboard[SDL_SCANCODE_D] - SDL_Input::keyboard[SDL_SCANCODE_A]);
    transform->UpdateVelocity(forward, right);

    transform->rotation = glm::vec3(SDL_Input::xRel, SDL_Input::yRel, 0);
    transform->Update(dt);

	view = glm::lookAt(transform->position, transform->position + transform->forward, transform->up);
}

Camera Camera::operator=(const Camera& c) {
    if (this == &c) return *this;

	this->proj = c.proj;
	this->view = c.view;
    this->fov = c.fov;
    this->aspect_ratio = c.aspect_ratio;
    this->near_plane = c.near_plane;
    this->far_plane = c.far_plane;

	this->transform = c.transform;

    return *this;
}