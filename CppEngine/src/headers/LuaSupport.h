
#ifndef LUA_SUPPORT_H_
#define LUA_SUPPORT_H_

#include <string>

#include "lua-5.3.5/src/lua.hpp"
#include "sol/sol.hpp"
#include "Globals.h"

void luaSetup(sol::state& L);

// output functionName(lua_State* luaState);

int addPointLight(const float&, const float&, const float&, const float&, const float&, const float&, const float&, const float&);
int addModel();
int addInstance(const std::string&);
int placeInstance(const int&, const float&, const float&, const float&);

#endif // LUA_SUPPORT_H_