
#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "Component.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Transform : public Component {

public:
    Transform();
    ~Transform();
    Transform* clone() const;

    Transform(const Transform&);
    Transform& operator=(const Transform&);

    void UpdatePosition(const glm::vec3&);
    void UpdateVelocity(const float&, const float&);
    void Update(const float&);

    void SetPosition(const glm::vec3& p) { 
        position = p;
        model = glm::translate(model, position);
    }

    glm::mat4 model;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 velocity;
};
#endif
