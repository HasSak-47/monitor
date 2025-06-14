#include <iostream>
#include <lua.h>
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

static int _buffer_get_sub(lua_State* L) {
    // Get the parent Buffer userdata
    Buffer* parent = static_cast<Buffer*>(
        luaL_checkudata(L, 1, "Buffer"));

    // Get arguments
    size_t x = luaL_checkinteger(L, 2);
    size_t y = luaL_checkinteger(L, 3);
    size_t w = luaL_checkinteger(L, 4);
    size_t h = luaL_checkinteger(L, 5);

    // Call C++ method
    Buffer sub = parent->get_sub_buffer(x, y, w, h);

    // Create new Lua userdata for the sub-buffer
    void* mem = lua_newuserdata(L, sizeof(Buffer));
    new (mem)
        Buffer(std::move(sub)); // Use move constructor

    // Set metatable
    luaL_getmetatable(L, "Buffer");
    lua_setmetatable(L, -2);

    return 1; // Return the new sub-buffer
}

static int _buffer_gc(lua_State* L) {
    Buffer* buf = static_cast<Buffer*>(
        luaL_checkudata(L, 1, "Buffer"));
    buf->~Buffer(); // Call destructor if needed
    return 0;
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

lua::LuaWidget::LuaWidget(int ref) : _lua_ref(ref) {}
lua::LuaWidget::~LuaWidget() {
    if (_lua_ref != 0) {
        luaL_unref(lua::L, LUA_REGISTRYINDEX, _lua_ref);
    }
}

void lua::LuaWidget::update() {};

void lua::LuaWidget::render(
    Buffer& buf, size_t tick) const {
    lua_pushlightuserdata(lua::L, &buf);

    lua_rawgeti(lua::L, LUA_REGISTRYINDEX, _lua_ref);
    lua_getfield(lua::L, -1, "render");
    if (lua_isfunction(lua ::L, -1)) {

        lua_pushlightuserdata(lua::L, &buf);
        lua_pushinteger(lua::L, tick);

        if (lua_pcall(lua::L, 2, 0, 0) != LUA_OK) {
            const char* err = lua_tostring(lua::L, -1);
            std::cerr << "LuaWidget render error: " << err
                      << std::endl;
            lua_pop(lua::L, 1);
        }
        return;
    }
    lua_pop(lua::L, 1);

    if (lua_getmetatable(lua::L, -1)) {
        lua_getfield(lua::L, -1, "render");
        if (lua_isfunction(lua::L, -1)) {
            lua_pushvalue(lua::L, -3);
            lua_pushlightuserdata(lua::L, &buf);
            lua_pushinteger(lua::L, tick);

            if (lua_pcall(lua::L, 3, 0, 0) != LUA_OK) {
                // Handle error
                const char* err = lua_tostring(lua::L, -1);
                std::cerr
                    << "LuaWidget render error: " << err
                    << std::endl;
                lua_pop(lua::L, 1);
            }
        }
        else {
            lua_pop(lua::L, 1);
        }
        lua_pop(lua::L, 1);
    }
}

int _widget_create(lua_State* L) {
    size_t size = lua_gettop(L);
    switch (size) {
    case 0:
        lua_createtable(L, 0, 0);
        break;
    case 1:
        if (!lua_istable(L, 1)) {
            luaL_error(L,
                "widget.new expects a table or no arguments");
        }
        break;
    default:
        return luaL_error(L,
            "called widget.new with more than 1 argument");
    }

    int ref = luaL_ref(L, 1);

    lua::LuaWidget** ud = (lua::LuaWidget**)lua_newuserdata(
        L, sizeof(lua::LuaWidget*));
    *ud = new lua::LuaWidget(ref);

    luaL_getmetatable(L, "Widget");
    lua_setmetatable(L, -2);

    return 1;
}

static int _widget_new(lua_State* L) {
    size_t size = lua_gettop(L);
    switch (size) {
    case 0:
        lua_createtable(L, 0, 0);
        break;
    case 1:
        if (!lua_istable(L, 1)) {
            luaL_error(L,
                "widget.new expects a table or no arguments");
        }
        break;
    default:
        return luaL_error(L,
            "called widget.new with more than 1 argument");
    }

    luaL_getmetatable(L, "Widget");
    lua_setmetatable(L, -2);

    return 1;
}

int _widget_gc(lua_State* L) {
    lua::LuaWidget** widgetPtr =
        (lua::LuaWidget**)luaL_checkudata(L, 1, "Widget");
    delete *widgetPtr;
    return 0;
}

static int _widget_render(lua_State* L) {
    // empty by default
    return 0;
}

static int _widget_update(lua_State* L) {
    // empty by default
    return 0;
}

static void init_widget_metatable() {
    static const luaL_Reg widget_methods[] = {
        {   "new",    _widget_new},
        {"render", _widget_render},
        {"update", _widget_update},
        {  "__gc",     _widget_gc},
        {"__call", _widget_create},
        {    NULL,           NULL}
    };

    using namespace lua;
    luaL_newmetatable(L, "Widget");

    luaL_setfuncs(L, widget_methods, 0);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_newtable(L);
    luaL_setfuncs(L, widget_methods, 0);
    lua_setglobal(L, "widget");
}

static void init_buffer_metatable() {
    luaL_newmetatable(lua::L, "Buffer");

    lua_pushcfunction(lua::L, _buffer_set);
    lua_setfield(lua::L, -2, "set");

    lua_pushcfunction(lua::L, _buffer_get_size);
    lua_setfield(lua::L, -2, "get_size");

    lua_pushcfunction(lua::L, _buffer_get_sub);
    lua_setfield(lua::L, -2, "get_sub");

    lua_pushcfunction(lua::L, _buffer_gc);
    lua_setfield(lua::L, -2, "__gc");

    lua_pushvalue(lua::L, -1);
    lua_setfield(lua::L, -2, "__index");

    lua_pop(lua::L, 1);
}

void lua::lua_init() {
    using namespace lua;
    L = luaL_newstate();
    luaL_openlibs(L);
    // luaL_requiref(L, "base", luaopen_base, true);
    // luaL_requiref(L, "math", luaopen_math, true);
    // luaL_requiref(L, "table", luaopen_table, true);
    // luaL_requiref(L, "package", luaopen_package, true);
    // luaL_requiref(L, "string", luaopen_string, true);

    init_buffer_metatable();
    init_widget_metatable();
};
lua::LuaWidget* create_from_file(std::string path) {
    if (luaL_loadstring(lua::L, path.c_str()) ||
        lua_pcall(lua::L, 0, 1, 0)) {
        std::cerr << "Error loading script: "
                  << lua_tostring(lua::L, -1) << std::endl;
        lua_pop(lua::L, 1);
        return nullptr;
    }

    if (!lua_istable(lua::L, -1)) {
        std::cerr << "Script must return a table"
                  << std::endl;
        lua_pop(lua::L, 1);
        return nullptr;
    }

    lua_getmetatable(lua::L, -1);

    lua_pop(lua::L, lua_gettop(lua::L));
}
