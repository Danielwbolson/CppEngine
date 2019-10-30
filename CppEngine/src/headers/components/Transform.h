
#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "Component.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Transform : public Component {

public:
    Transform();
	Transform(const glm::vec3&, const glm::vec3&, const glm::vec3&);
    Transform* clone() const;

    Transform(const Transform&);
    Transform& operator=(const Transform&);

    void UpdateVelocity(const float&, const float&);
	void UpdateRotation(const glm::vec3&);

	void Rotate(const float&, const glm::vec3& axis = glm::vec3(0, 1, 0));
	void Translate(const glm::vec3&);
	void Scale(const glm::vec3&);

    void Update(const float&);

    void SetPosition(const glm::vec3& p) { 
        position = p;
        model = glm::translate(model, position);
    }

    glm::mat4 model;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale = glm::vec3(1, 1, 1);

    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 velocity;
};
#endif
