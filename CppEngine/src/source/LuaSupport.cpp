
#include "LuaSupport.h"

#include "Scene.h"
#include "Light.h"
#include "PointLight.h"

void luaSetup(sol::state& L) {
	L.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);

	L.set_function("addPointLight", &addPointLight);
	L.set_function("addModel", &addModel);
	L.set_function("addInstance", &addInstance);
	L.set_function("placeInstance", &placeInstance);
	L.set_function("scaleInstance", &scaleInstance);

}


// C++ implementations
// output functionName(lua_State* luaState) {}

int addPointLight(const float& r, const float& g, const float& b, const float& a, const float& x , const float& y, const float& z, const float& w) {
	mainScene->lights.push_back(new PointLight(glm::vec4(r, g, b, a), glm::vec4(x, y, z, w)));
	return static_cast<int>(mainScene->lights.size() - 1);
}

int addModel() {
	return 0;
}

int addInstance(const std::string& gameObjectName) {
	mainScene->instances.push_back(new GameObject(*(mainScene->FindGameObject(gameObjectName))));

	int index = static_cast<int>(mainScene->instances.size() - 1);
	GameObject* g = mainScene->instances[index];
	for (int i = 0; i < g->components.size(); i++) {
		g->components[i]->gameObject = g;
	}
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