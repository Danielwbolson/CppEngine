
#ifndef CAMERA_H_
#define CAMERA_H_

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "Transform.h"

#include <vector>

class Camera {

public:
    Camera();
	~Camera();

    Camera(const glm::vec3&, const glm::vec3&, const glm::vec3&, 
		const float&, const float&, const float&);

    void Update(const float&);
	void UpdateFrustumPlanes();

    Camera operator=(const Camera&);

    Transform* transform;

    glm::mat4 view;
    glm::mat4 proj;

	std::vector<glm::vec4, MemoryAllocator<glm::vec4> > frustumPlanes;

    float fov, aspect_ratio;
    float near_plane, far_plane;
};

#endif