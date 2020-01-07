
#include "Transform.h"
#include "GameObject.h"

#define _USE_MATH_DEFINES
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include <math.h>

Transform::Transform() {
    componentType = "transform";
    position = glm::vec3(0, 0, 0);
	velocity = glm::vec3(0, 0, 0);
	scale = glm::vec3(1, 1, 1);

    rotation = glm::vec3(3.14159, 0, 0);

	model = glm::mat4(1.0);

	forward = glm::vec3(0, 0, -1);
	up = glm::vec3(0, 1, 0);
	right = glm::vec3(1, 0, 0);
}

Transform::Transform(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& u) {
	componentType = "transform";
	position = pos;
	velocity = glm::vec3(0, 0, 0);
	scale = glm::vec3(1, 1, 1);

	// Need to match with given directions
	glm::vec3 tempVec = glm::normalize(glm::vec3(dir.x, 0, dir.z));
	float xRot = glm::orientedAngle(tempVec, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	glm::vec3 dirXRot = glm::rotate(tempVec, xRot, glm::vec3(0, 1, 0));

	float yRot = glm::angle(tempVec, dir);

	rotation = glm::vec3(xRot, yRot, 0);

	model = glm::mat4(1.0);
	Rotate(-rotation.x, glm::vec3(0, 1, 0));
	Rotate(-rotation.y, glm::vec3(1, 0, 0));
	Translate(position);

	forward = -glm::vec3(model[2]);
	up = glm::vec3(model[1]);
	right = -glm::vec3(model[0]);
}

Transform* Transform::clone() const {
	return memoryManager->Allocate<Transform>(*this);
}

Transform::Transform(const Transform& t) {

    this->componentType = t.componentType;
    this->position = t.position;
	this->velocity = t.velocity;
    this->scale = t.scale;

	this->rotation = t.rotation;
    
	model = t.model;

    this->forward = t.forward;
    this->right = t.right;
    this->up = t.up;

    gameObject = t.gameObject;
}

Transform& Transform::operator=(const Transform& t) {
    if (this == &t) return *this;

	this->componentType = t.componentType;
    this->position = t.position;
	this->velocity = t.velocity;
	this->scale = t.scale;

    this->rotation = t.rotation;

	model = t.model;

    this->forward = t.forward;
    this->right = t.right;
    this->up = t.up;

    gameObject = t.gameObject;
    return *this;
}

void Transform::UpdateVelocity(const float& f, const float& r) {
    velocity = forward * f + right * r;
}

void Transform::UpdateRotation(const glm::vec3& rot) {
	rotation += rot;
	rotation.y = glm::clamp(rotation.y, (float)-M_PI / 2.0f * 0.99f, (float)M_PI / 2.0f * 0.99f);
}

void Transform::Rotate(const float& value, const glm::vec3& axis) {
	model = glm::rotate(model, value, axis);
}

void Transform::Translate(const glm::vec3& amount) {
	model = glm::translate(model, amount);
}

void Transform::Scale(const glm::vec3& amount) {
	model = glm::scale(model, amount);
}

void Transform::Update(const float& dt) {
    position += velocity * dt;

	model = glm::mat4(1.0);
	Translate(position + velocity * dt);
	Rotate(-rotation.x, glm::vec3(0, 1, 0));
	Rotate(-rotation.y, glm::vec3(1, 0, 0));
	Scale(scale);
	
    forward = -glm::vec3(model[2]);
    up = glm::vec3(model[1]);
    right = glm::vec3(model[0]);
}
