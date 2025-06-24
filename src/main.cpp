#include <algorithm>
#include <lua.hpp>

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
        { "mem", Value::integer((int64_t)p->total())},
    });
}

void set_process_table(ly::render::lua::State& state) {
    using namespace ly::render::lua;

    int offset = state.get_data("offset").as_integer();
    bool show_kernel =
        state.get_data("show_kernel").as_boolean();

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

    std::sort(procs.begin(), procs.end(), _sort_mem);

    // Clamp offset
    if (offset < 0)
        offset = 0;
    if ((size_t)offset > procs.size())
        offset = procs.size();

    state.set_data("offset", Value::integer(offset));

    // Optional: define max entries per page
    constexpr int page_size = 100;
    size_t end = std::min<size_t>(page_size, procs.size());

    Value::ArrayType array;
    array.reserve(end - offset);
    for (size_t i = offset; i < end; ++i) {
        array.push_back(from_process(procs[i]));
    }

    state.set_data(
        "processes", Value::array(std::move(array)));
    state.set_data("process_total",
        Value::integer((int64_t)procs.size()));
}

int main(int argc, char* argv[]) {
    using namespace std::chrono;
    using namespace ly;

    constexpr auto tick_duration = 16.6666ms;
    render::Window win;

    ly::render::lua::State state;
    auto widget = state.from_file("init.lua");

    state.set_data(
        "offset", render::lua::Value::integer(0));

    float fps    = 0;
    float tdelta = 0;
    size_t tick  = 0;
    char cbuf    = 0;

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

        set_process_table(state);

        widget.update();
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

        // Optional: print elapsed time for debugging
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
