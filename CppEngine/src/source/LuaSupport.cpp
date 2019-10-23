
#include "LuaSupport.h"

void luaSetup(lua_State* L) {
	luaL_openlibs(L);

	// lua_register(luaState, "functionName", functionName);

	lua_register(L, "addLight", addLight);
	lua_register(L, "addModel", addModel);
	lua_register(L, "addInstance", addInstance);

}


// C++ implementations
// output functionName(lua_State* luaState) {}

int addLight(lua_State* L) {
	return 0;
}

int addModel(lua_State* L) {
	return 0;
}

int addInstance(lua_State* L) {
	return 0;
}