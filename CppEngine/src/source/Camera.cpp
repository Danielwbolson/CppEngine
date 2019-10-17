
#include "Camera.h"
#include "SDL_Static_Helper.h"

Camera::Camera() {
    transform = new Transform();
	frustumPlanes = std::vector<glm::vec4>(6);
}

Camera::~Camera() {
	delete transform;
}

Camera::Camera(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up, 
    const int& w, const int& h, const float& f, const float& np, const float& fp) {

	transform = new Transform(pos, glm::normalize(dir), glm::normalize(up));

    fov = f;
    near_plane = np;
    far_plane = fp;
    aspect_ratio = w / (float)h;

    proj = glm::perspective(fov, aspect_ratio, near_plane, far_plane);
	view = glm::lookAt(transform->position, transform->position + transform->forward, transform->up);

	frustumPlanes = std::vector<glm::vec4>(6);
	UpdateFrustumPlanes();
}

void Camera::Update(const float& dt) {
    float forward = 4.0f * (SDL_Input::keyboard[SDL_SCANCODE_W] - SDL_Input::keyboard[SDL_SCANCODE_S]);
    float right = 4.0f * (SDL_Input::keyboard[SDL_SCANCODE_D] - SDL_Input::keyboard[SDL_SCANCODE_A]);
    
	// If our camera has not changed at all (mouse movement, keyboard movement)
	if (forward == 0 && right == 0 && SDL_Input::xRel == 0 && SDL_Input::yRel == 0) {
		return;
	}

	// Otherwise, we need to update our transform, view, and frustum planes
	transform->UpdateVelocity(forward, right);
	transform->UpdateRotation(glm::vec3(SDL_Input::xRel, SDL_Input::yRel, 0));
    transform->Update(dt);

	view = glm::lookAt(transform->position, transform->position + transform->forward, transform->up);
	UpdateFrustumPlanes();
}

void Camera::UpdateFrustumPlanes() {
	glm::mat4 projViewMat = proj * view;

	/* http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf */
	glm::vec4 left = glm::vec4(
		projViewMat[0][3] + projViewMat[0][0],
		projViewMat[1][3] + projViewMat[1][0],
		projViewMat[2][3] + projViewMat[2][0],
		projViewMat[3][3] + projViewMat[3][0]);

	glm::vec4 right = glm::vec4(
		projViewMat[0][3] - projViewMat[0][0],
		projViewMat[1][3] - projViewMat[1][0],
		projViewMat[2][3] - projViewMat[2][0],
		projViewMat[3][3] - projViewMat[3][0]);

	glm::vec4 bottom = glm::vec4(
		projViewMat[0][3] + projViewMat[0][1],
		projViewMat[1][3] + projViewMat[1][1],
		projViewMat[2][3] + projViewMat[2][1],
		projViewMat[3][3] + projViewMat[3][1]);

	glm::vec4 top = glm::vec4(
		projViewMat[0][3] - projViewMat[0][1],
		projViewMat[1][3] - projViewMat[1][1],
		projViewMat[2][3] - projViewMat[2][1],
		projViewMat[3][3] - projViewMat[3][1]);

	glm::vec4 nearPlane = glm::vec4(
		projViewMat[0][2],
		projViewMat[1][2],
		projViewMat[2][2],
		projViewMat[3][2]);

	glm::vec4 farPlane = glm::vec4(
		projViewMat[0][3] - projViewMat[0][2],
		projViewMat[1][3] - projViewMat[1][2],
		projViewMat[2][3] - projViewMat[2][2],
		projViewMat[3][3] - projViewMat[3][2]);

	frustumPlanes[0] = left;
	frustumPlanes[1] = right;
	frustumPlanes[2] = bottom;
	frustumPlanes[3] = top;
	frustumPlanes[4] = nearPlane;
	frustumPlanes[5] = farPlane;

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