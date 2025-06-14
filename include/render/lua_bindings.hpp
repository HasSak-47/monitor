#ifndef __RENDER_LUA_BINDINGS_HPP__
#define __RENDER_LUA_BINDINGS_HPP__

#include <lua.hpp>
#include "render/widgets.hpp"

namespace ly::render::lua {
extern lua_State* L;

void lua_init();

class LuaWidget : public widgets::Widget {
private:
    int _lua_ref = 0;

public:
    LuaWidget(int ref);
    ~LuaWidget() override;

    void update() override;
    void render(
        Buffer& buf, size_t tick = 0) const override;
};

LuaWidget* create_from_file(std::string path);

} // namespace ly::render::lua

#endif
