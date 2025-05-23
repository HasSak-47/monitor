#include <lua.hpp>

lua_State* L = nullptr;

int main() {
    L = luaL_newstate();
    luaL_requiref(L, "base", luaopen_base, true);
    luaL_requiref(L, "math", luaopen_math, true);
    luaL_requiref(L, "table", luaopen_table, true);
    luaL_requiref(L, "package", luaopen_package, true);
    luaL_requiref(L, "string", luaopen_string, true);

    lua_close(L);
    return 0;
}
