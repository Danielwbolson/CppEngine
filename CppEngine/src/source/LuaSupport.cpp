
#include "LuaSupport.h"

#define _USE_MATH_DEFINES
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "Scene.h"
#include "Light.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "GameObject.h"
#include "ModelRenderer.h"
#include "Model.h"
#include "Material.h"

void luaSetup(sol::state& L) {
	L.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);

	L.set_function("addPointLight", &addPointLight);
	L.set_function("addDirectionalLight", &addDirectionalLight);
	L.set_function("addModel", &addModel);
	L.set_function("addInstance", &addInstance);
	L.set_function("placeInstance", &placeInstance);
	L.set_function("scaleInstance", &scaleInstance);
	L.set_function("rotateInstance", &rotateInstance);
	L.set_function("rotateSunX", &rotateSunX);

}


// C++ implementations
// output functionName(lua_State* luaState) {}

int addPointLight(const float& r, const float& g, const float& b, const float& x , const float& y, const float& z) {
	mainScene->lights.push_back(MemoryManager::Allocate<PointLight>(
		glm::vec3(r, g, b), 
		glm::vec4(x, y, z, 1))
	);

	return static_cast<int>(mainScene->lights.size() - 1);
}

int addDirectionalLight(const float& r, const float& g, const float& b, const float& dx, const float& dy, const float& dz) {
	mainScene->lights.push_back(MemoryManager::Allocate<DirectionalLight>(
		glm::vec4(r, g, b, 1),
		glm::vec4(glm::normalize(glm::vec3(dx, dy, dz)), 1))
	);
	return static_cast<int>(mainScene->lights.size() - 1);
}

int addModel() {
	return 0;
}

int addInstance(const std::string& gameObjectName) {
	GameObject* g = MemoryManager::Allocate<GameObject>(*(mainScene->FindGameObject(gameObjectName)));
	for (int i = 0; i < g->components.size(); i++) {
		g->components[i]->gameObject = g;
	}

	mainScene->instances.push_back(g);

	return static_cast<int>(mainScene->instances.size() - 1);
}

int scaleInstance(const int& index, const float& x, const float& y, const float& z) {
	mainScene->instances[index]->transform->scale = glm::vec3(x, y, z);
	return 1;
}

int placeInstance(const int& index, const float& x, const float& y, const float& z) {
	mainScene->instances[index]->transform->SetPosition(glm::vec3(x, y, z));
	return 1;
}

int rotateInstance(const int& index, const float& xRot, const float& yRot) {
	mainScene->instances[index]->transform->rotation = glm::vec3(xRot, yRot, 0);
	return 1;
}

int rotateSunX(const int& index, const float& angle) {
	DirectionalLight* sun = (DirectionalLight*)mainScene->lights[index];
	sun->direction = glm::normalize(glm::vec4(glm::rotate(glm::vec3(sun->direction), angle, glm::vec3(1, 0, 0)), 0));
	return 1;
}