
#include "LuaSupport.h"

#include "Scene.h"
#include "Light.h"
#include "PointLight.h"

void luaSetup(sol::state& L) {
	L.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);

	L.set_function("addPointLight", &addPointLight);
	L.set_function("addModel", &addModel);
	L.set_function("addInstance", &addInstance);

}


// C++ implementations
// output functionName(lua_State* luaState) {}

int addPointLight(const float& r, const float& g, const float& b, const float& a, const float& x , const float& y, const float& z, const float& w) {
	mainScene->lights.push_back(new PointLight(glm::vec4(r, g, b, a), glm::vec4(x, y, z, w)));
	return 1;
}

int addModel() {
	return 0;
}

int addInstance() {
	return 0;
}