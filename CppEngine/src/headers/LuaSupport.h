
#ifndef LUA_SUPPORT_H_
#define LUA_SUPPORT_H_

#include "lua-5.3.5/src/lua.hpp"

void luaSetup(lua_State* L);

// output functionName(lua_State* luaState);

int addLight(lua_State* L);
int addModel(lua_State* L);
int addInstance(lua_State* L);

#endif // LUA_SUPPORT_H_