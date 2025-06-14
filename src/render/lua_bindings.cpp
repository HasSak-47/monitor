#include <iostream>
#include <render/buffer.hpp>
#include <render/lua_bindings.hpp>
#include <render/widgets.hpp>
#include <stdexcept>

lua_State* ly::render::lua::L = nullptr;

using namespace ly::render;

static int _buffer_get_size(lua_State* L) {
    Buffer* data = (Buffer*)lua_touserdata(L, 1);

    lua_pushinteger(L, data->width());
    lua_pushinteger(L, data->height());

    return 2;
}

static int _buffer_set(lua_State* L) {
    Buffer* data = (Buffer*)lua_touserdata(L, 1);
    if (data == NULL) {
        throw std::runtime_error("data is null?");
    }

    size_t x = lua_tointeger(L, 2);
    size_t y = lua_tointeger(L, 3);

    // map [1, b] to [0, b)
    if (x <= 0)
        x = 1;
    if (y <= 0)
        y = 1;

    x -= 1;
    y -= 1;

    // get first character
    char c = lua_tostring(L, 4)[0];
    lua_getfield(L, 5, "type");
    lua_getfield(L, 5, "r");
    lua_getfield(L, 5, "g");
    lua_getfield(L, 5, "b");

    std::string ty = lua_tostring(L, 6);
    int r          = lua_tointeger(L, 7);
    int g          = lua_tointeger(L, 8);
    int b          = lua_tointeger(L, 9);

    ConsoleColor col = ConsoleColor::WHITE;

    if (ty == "bit") {
        r = r & 1 << 0;
        g = g & 1 << 1;
        b = b & 1 << 2;

        col = ConsoleColor(r | g | b);
    }

    else if (ty == "8bit") {
        r = r & 0xff;
        g = g & 0xff;
        b = b & 0xff;

        col = ConsoleColor(Color<ly::u8>(r, g, b));
    }
    auto& unit = data->get(x, y);

    unit.chr = c;
    unit.col = col;

    return 0;
}

/*
static int _buffer_get(lua_State* L){
    return 1;
}

class LuaWidget : public widgets::Widget {
private:
    int _lua_ref = 0;

public:
    LuaWidget(int ref) : _lua_ref(ref) {}
    ~LuaWidget() override {}

    void update() override {};
    void render(Buffer& buf) const override {
        // TODO: get lua object back and pass buf to it
    };
};

std::vector<LuaWidget> _widgets;

static int _widget_new(lua_State* L) {
    size_t size = lua_gettop(L);
    switch (size) {
    case 0:
        lua_createtable(L, 0, 0);
        break;
    case 1:
        break;
    default:
        throw std::runtime_error(
            "called widget new with more than 1 argument");
        return 1;
    }

    luaL_getmetatable(L, "Widget");
    lua_setmetatable(L, -2);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    _widgets.push_back({ref});

    return 1;
}

static int _widget_render(lua_State* L) {
    // empty by default
    return 0;
}

static int _widget_update(lua_State* L) {
    // empty by default
    return 0;
}

static int init_widget_metatable() {
    using namespace lua;
    luaL_newmetatable(L, "Widget");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, _widget_render);
    lua_setfield(L, -2, "render");

    lua_pushcfunction(L, _widget_update);
    lua_setfield(L, -2, "update");

    lua_pushcfunction(L, _widget_new);
    lua_setfield(L, -2, "new");

    return 1;
}
*/

void lua::lua_init() {
    using namespace lua;
    L = luaL_newstate();
    luaL_openlibs(L);
    // luaL_requiref(L, "base", luaopen_base, true);
    // luaL_requiref(L, "math", luaopen_math, true);
    // luaL_requiref(L, "table", luaopen_table, true);
    // luaL_requiref(L, "package", luaopen_package, true);
    // luaL_requiref(L, "string", luaopen_string, true);

    luaL_newmetatable(L, "Buffer");

    lua_pushcfunction(L, _buffer_set);
    lua_setfield(L, -2, "set");

    lua_pushcfunction(L, _buffer_get_size);
    lua_setfield(L, -2, "get_size");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
};
