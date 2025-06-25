#include <lua.hpp>

#include <algorithm>
#include <chrono>
#include <functional>
#include <thread>

#include <cstdio>
#include <cstdlib>

#include <termios.h>
#include <unistd.h>

#include <ly/render/buffer.hpp>
#include <ly/render/lua_bindings.hpp>
#include <ly/render/utils.hpp>
#include <ly/render/widgets.hpp>
#include <ly/render/window.hpp>

#include <system/system.hpp>

using _Sorter = std::function<bool(
    const sys::Process*&, const sys::Process*&)>;

static inline bool _sort_name(
    const sys::Process*& a, const sys::Process*& b) {
    return a->_stat.name < b->_stat.name;
}

static inline bool _sort_pid(
    const sys::Process*& a, const sys::Process*& b) {
    return a->_stat.pid < b->_stat.pid;
}

static inline bool _sort_mem(
    const sys::Process*& a, const sys::Process*& b) {
    return a->total() > b->total();
}

ly::render::lua::Value from_process(
    const sys::Process*& p) {
    using namespace ly::render::lua;
    return Value::map({
        {"name",        Value::string(p->_stat.name)},
        { "pid",        Value::integer(p->_stat.pid)},
        {"ppid", Value::integer(p->_stat.parent_pid)},
        { "mem", Value::integer((int64_t)p->total())},
        { "cmd",              Value::string(p->_cmd)},
    });
}

void set_process_table(ly::render::lua::State& state) {
    using namespace ly::render::lua;

    bool show_kernel =
        state.get_data("show_kernel").as_boolean();

    std::string sorting =
        state.get_data("sorting").as_string();

    _Sorter sorter = _sort_mem;
    if (sorting == "pid") {
        sorter = _sort_pid;
    }
    else if (sorting == "name") {
        sorter = _sort_name;
    }
    else {
        sorter = _sort_mem;
    }

    std::vector<const sys::Process*> procs;
    for (const auto& [_, proc] : sys::sys.get_processes()) {
        procs.push_back(&proc);
    }

    if (!show_kernel) {
        auto val = std::find_if(procs.begin(), procs.end(),
            [](const auto& a) { return a->is_kernel(); });

        while (val != procs.end()) {
            val = procs.erase(val);
            val = std::find_if(
                val, procs.end(), [](const auto& a) {
                    return a->is_kernel();
                });
        }
    }

    std::sort(procs.begin(), procs.end(), sorter);

    Value::ArrayType array;

    for (size_t i = 0; i < procs.size(); ++i) {
        array.push_back(from_process(procs[i]));
    }

    state.set_data(
        "processes", Value::array(std::move(array)));
    state.set_data("process_total",
        Value::integer((int64_t)procs.size()));
}

std::string init_path = "./init.lua";

int main(int argc, char* argv[]) {
    using namespace std::chrono;
    using namespace ly;
    using namespace ly::render;

    constexpr auto tick_duration = 16.6666ms;

    Window win;
    lua::State state;

    state.set_function("set_color", [&](lua_State* L) {
        std::string type = lua_tostring(L, 1);

        render::ConsoleColor& color = (type == "fg")
                                          ? win.default_fc
                                          : win.default_bc;
        lua_getfield(L, 2, "type");
        lua_getfield(L, 2, "r");
        lua_getfield(L, 2, "g");
        lua_getfield(L, 2, "b");

        std::string ty = lua_tostring(L, -4);
        int r          = lua_tointeger(L, -3);
        int g          = lua_tointeger(L, -2);
        int b          = lua_tointeger(L, -1);
        lua_pop(L, 4);

        if (ty == "bit") {
            r     = (r & 1) << 0;
            g     = (g & 1) << 1;
            b     = (b & 1) << 2;
            color = render::ConsoleColor(r | g | b);
        }
        else if (ty == "8bit") {
            color =
                render::ConsoleColor(render::Color<ly::u8>(
                    r & 0xff, g & 0xff, b & 0xff));
        }

        return 0;
    });
    win.init_buffer();

    auto widget = state.from_file(init_path);

    state.set_data("offset", lua::Value::integer(0));
    state.set_data(
        "show_kernel", lua::Value::boolean(true));
    state.set_data("sorting", lua::Value::string("memory"));
    state.set_data("reload", lua::Value::boolean(false));
    state.set_data("debug", lua::Value::boolean(false));
    state.set_data("help", lua::Value::boolean(false));

    float fps    = 0;
    float tdelta = 0;
    size_t tick  = 0;
    char cbuf    = 0;

    enter_alternate_screen();
    set_raw_mode();

    while (!state.should_exit()) {
        auto t_start = high_resolution_clock::now();
        reset_cursor();
        win.resize();
        if (state.get_data("reload").as_boolean() == true) {
            widget = state.from_file(init_path);
            state.set_data(
                "reload", lua::Value::boolean(false));
        }

        if (read(STDIN_FILENO, &cbuf, 1) > 0) {
            state.press(cbuf);
        }

        state.set_data(
            "total_mem", lua::Value::integer(
                             (int64_t)sys::sys._max_mem));

        state.set_data("av_mem",
            lua::Value::integer((int64_t)sys::sys._av_mem));

        state.set_data("cached_mem",
            lua::Value::integer(
                (int64_t)sys::sys._cached_mem));

        state.set_data("width",
            lua::Value::integer((int64_t)win.width()));

        state.set_data("heigth",
            lua::Value::integer((int64_t)win.height()));

        set_process_table(state);

        if (tick % 10 == 0) {
            widget.update();
        }
        win.get_buf().render_widget(widget);
        win.render();

        // tick rate calc
        auto t_now = high_resolution_clock::now();
        auto delta = t_now - t_start;

        // wait for end of tick
        if (delta < tick_duration) {
            tdelta =
                duration_cast<milliseconds>(delta).count();
            std::this_thread::sleep_for(
                tick_duration - delta);
        }

        auto t_end = high_resolution_clock::now();
        auto total_elapsed =
            duration_cast<milliseconds>(t_end - t_start);
        fps = 1000.0 / total_elapsed.count();
        tick++;
        state.set_data("tick",
            render::lua::Value::integer((int64_t)tick));
        state.set_data("tdelta",
            render::lua::Value::float_val((double)tdelta));
        state.set_data("fps",
            render::lua::Value::float_val((double)fps));
    }

    ly::render::leave_alternate_screen();
    ly::render::unset_raw_mode();
    return 0;
}
