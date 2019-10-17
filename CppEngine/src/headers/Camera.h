
#ifndef CAMERA_H_
#define CAMERA_H_

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "Transform.h"

class Camera {

public:
    Camera();
	~Camera();

    Camera(const glm::vec3&, const glm::vec3&, const glm::vec3&, 
        const int&, const int&, const float&, const float&, const float&);

    void Update(const float&);

    Camera operator=(const Camera&);

    Transform* transform;

    glm::mat4 view;
    glm::mat4 proj;

    float fov, aspect_ratio;
    float near_plane, far_plane;
};

#endif