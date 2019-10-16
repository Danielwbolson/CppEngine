
#ifndef CAMERA_H_
#define CAMERA_H_

#include "glm/glm.hpp"
#include "Transform.h"

class Camera {

public:
    Camera();
    Camera(const glm::vec3&, const glm::vec3&, 
        const int&, const int&, const float&, const float&, const float&);

    void Update(const float&);

    Camera operator=(const Camera&);

    Transform* transform;

    glm::mat4 view;
    glm::mat4 proj;

    glm::vec3 lookAt;
    glm::vec3 up;

    float fov, aspect_ratio;
    float near_plane, far_plane;
};

#endif