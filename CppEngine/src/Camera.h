
#ifndef CAMERA_H_
#define CAMERA_H_

#define GLM_FORCE_RADIANS //ensure we are using radians
#include "../glad/glad.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include "Transform.h"
#include "SDL_Static_Helper.h"

#include <stdio.h>

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