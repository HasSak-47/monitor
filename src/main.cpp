#include <chrono>
#include <lua.hpp>
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

int main(int argc, char* argv[]) {
    using namespace std::chrono;
    using namespace ly;

    constexpr auto tick_duration = 16.6666ms;
    render::Window win;

    ly::render::lua::State state;
    auto widget = state.from_file("init.lua");

    size_t tick = 0;
    char cbuf   = 0;
    auto val    = render::lua::Value::float_val(10.);
    ly::render::enter_alternate_screen();
    ly::render::set_raw_mode();

    while (!state.should_exit()) {
        auto t_start = high_resolution_clock::now();

        ly::render::reset_cursor();
        fflush(stdout);

        if (read(STDIN_FILENO, &cbuf, 1) > 0) {
            state.press(cbuf);
        }

        state.set_data(
            "total_mem", render::lua::Value::integer(
                             (int64_t)sys::sys._max_mem));

        state.set_data(
            "av_mem", render::lua::Value::integer(
                          (int64_t)sys::sys._av_mem));

        state.set_data("cached_mem",
            render::lua::Value::integer(
                (int64_t)sys::sys._cached_mem));

        widget.update();
        win.get_buf().render_widget(widget);
        win.render();

        // tick rate calc
        auto t_now = high_resolution_clock::now();
        auto delta = t_now - t_start;

        // wait for end of tick
        if (delta < tick_duration)
            std::this_thread::sleep_for(
                tick_duration - delta);

        // Optional: print elapsed time for debugging
        auto t_end = high_resolution_clock::now();
        auto total_elapsed =
            duration_cast<milliseconds>(t_end - t_start);
        float fps = 1000.0 / total_elapsed.count();
        tick++;
        state.set_data("tick",
            render::lua::Value::integer((int64_t)tick));
        state.set_data("fps",
            render::lua::Value::float_val((double)fps));
    }

    ly::render::leave_alternate_screen();
    ly::render::unset_raw_mode();
    return 0;
}
