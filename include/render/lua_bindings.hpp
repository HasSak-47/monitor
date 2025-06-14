#ifndef __RENDER_LUA_BINDINGS_HPP__
#define __RENDER_LUA_BINDINGS_HPP__

#include <lua.hpp>

namespace ly::render::lua {
extern lua_State* L;

void lua_init();

} // namespace ly::render::lua

#endif
